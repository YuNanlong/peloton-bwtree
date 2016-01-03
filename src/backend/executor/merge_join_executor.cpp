/*-------------------------------------------------------------------------
 *
 * merge_join.cpp
 * file description
 *
 * Copyright(c) 2015, CMU
 *
 * /peloton/src/executor/merge_join_executor.cpp
 *
 *-------------------------------------------------------------------------
 */

#include <vector>

#include "backend/common/types.h"
#include "backend/common/logger.h"
#include "backend/executor/logical_tile_factory.h"
#include "backend/executor/merge_join_executor.h"
#include "backend/expression/abstract_expression.h"
#include "backend/expression/container_tuple.h"

namespace peloton {
namespace executor {

/**
 * @brief Constructor for nested loop join executor.
 * @param node Nested loop join node corresponding to this executor.
 */
MergeJoinExecutor::MergeJoinExecutor(const planner::AbstractPlan *node,
                                     ExecutorContext *executor_context)
: AbstractJoinExecutor(node, executor_context) {
  join_clauses_ = nullptr;
}

bool MergeJoinExecutor::DInit() {
  auto status = AbstractJoinExecutor::DInit();
  if (status == false)
    return status;

  const planner::MergeJoinPlan &node = GetPlanNode<planner::MergeJoinPlan>();

  join_clauses_ = node.GetJoinClauses();

  if (join_clauses_ == nullptr)
    return false;

  return true;
}

/**
 * @brief Creates logical tiles from the two input logical tiles after applying
 * join predicate.
 * @return true on success, false otherwise.
 */
bool MergeJoinExecutor::DExecute() {
  LOG_TRACE("********** Merge Join executor :: 2 children \n"
      "left:: start: %lu, end: %lu, done: %d\n"
      "right:: start: %lu, end: %lu, done: %d\n", left_start_row, left_end_row, left_child_done_,
      right_start_row, right_end_row, right_child_done_);

  // Build outer join output when done
  if (right_child_done_ && left_child_done_) {
    return BuildOuterJoinOutput();
  }

  //===--------------------------------------------------------------------===//
  // Pick right and left tiles
  //===--------------------------------------------------------------------===//

  // Try to get next tile from RIGHT child
  if ( ( (right_child_done_ == false) && (right_start_row == right_end_row) ) || (left_child_done_ == true) ) {
    if (children_[1]->Execute() == false) {
      LOG_TRACE("Did not get right tile \n");
      right_child_done_ = true;
      // Try again
      return DExecute();
    }

    LOG_TRACE("Got right tile \n");

    auto right_tile = children_[1]->GetOutput();
    BufferRightTile(right_tile);

    right_start_row = 0;
    right_end_row = Advance(right_tile, right_start_row, false);
    LOG_TRACE("size of right tiles: %lu", right_result_tiles_.size());
  }

  // Try to get next tile from LEFT child
  if ( ( (left_child_done_ == false) && (left_start_row == left_end_row) ) || (right_child_done_ == true) ) {
    if (children_[0]->Execute() == false) {
      LOG_TRACE("Did not get left tile \n");
      left_child_done_ = true;
      // Try again
      return DExecute();
    }

    LOG_TRACE("Got left tile \n");

    auto left_tile = children_[0]->GetOutput();
    BufferLeftTile(left_tile);

    left_start_row = 0;
    left_end_row = Advance(left_tile, left_start_row, true);
    LOG_TRACE("size of left tiles: %lu", left_result_tiles_.size());
  }

  LogicalTile *left_tile = left_result_tiles_.back().get();
  LogicalTile *right_tile = right_result_tiles_.back().get();

  //===--------------------------------------------------------------------===//
  // Build Join Tile
  //===--------------------------------------------------------------------===//

  // Build output logical tile
  auto output_tile = BuildOutputLogicalTile(left_tile, right_tile);

  // Build position lists
  LogicalTile::PositionListsBuilder pos_lists_builder(left_tile, right_tile);

  while ((left_end_row > left_start_row) && (right_end_row > right_start_row)) {

    expression::ContainerTuple<executor::LogicalTile> left_tuple(
        left_tile, left_start_row);
    expression::ContainerTuple<executor::LogicalTile> right_tuple(
        right_tile, right_start_row);
    bool not_matching_tuple_pair = false;

    // Evaluate and compare the join clauses
    for (auto &clause : *join_clauses_) {
      auto left_value = clause.left_->Evaluate(&left_tuple, &right_tuple,
                                               nullptr);
      auto right_value = clause.right_->Evaluate(&left_tuple, &right_tuple,
                                                 nullptr);

      // Compare the values
      int comparison = left_value.Compare(right_value);

      // Left key < Right key, advance left
      if (comparison < 0) {
        LOG_TRACE("left < right, advance left");
        left_start_row = left_end_row;
        left_end_row = Advance(left_tile, left_start_row, true);
        not_matching_tuple_pair = true;
        break;
      }
      // Left key > Right key, advance right
      else if (comparison > 0) {
        LOG_TRACE("left > right, advance right");
        right_start_row = right_end_row;
        right_end_row = Advance(right_tile, right_start_row, false);
        not_matching_tuple_pair = true;
        break;
      }

      // Left key == Right key, go and check next join clause
    }

    // Atleast one of the join clauses don't match
    // One of the tile has been advanced
    if (not_matching_tuple_pair) {
      continue;
    }

    // Join clauses matched, try to match predicate
    LOG_TRACE("one pair of tuples matches join clause");

    // Join predicate exists
    if (predicate_ != nullptr) {
      if (predicate_->Evaluate(&left_tuple, &right_tuple, executor_context_)
          .IsFalse()) {
        // Join predicate is false. Advance both.
        left_start_row = left_end_row;
        left_end_row = Advance(left_tile, left_start_row, true);

        right_start_row = right_end_row;
        right_end_row = Advance(right_tile, right_start_row, false);
      }
    }

    // Sub tile matched, do a Cartesian product
    // Go over every pair of tuples in left and right logical tiles
    for (size_t left_tile_row_itr = left_start_row;
        left_tile_row_itr < left_end_row; left_tile_row_itr++) {

      for (size_t right_tile_row_itr = right_start_row;
          right_tile_row_itr < right_end_row; right_tile_row_itr++) {

        // Insert a tuple into the output logical tile
        pos_lists_builder.AddRow(left_tile_row_itr, right_tile_row_itr);

        RecordMatchedLeftRow(left_result_tiles_.size() - 1, left_tile_row_itr);
        RecordMatchedRightRow(right_result_tiles_.size() - 1, right_tile_row_itr);
      }
    }

    // Then, advance both
    left_start_row = left_end_row;
    left_end_row = Advance(left_tile, left_start_row, true);

    right_start_row = right_end_row;
    right_end_row = Advance(right_tile, right_start_row, false);
  }

  // Check if we have any join tuples.
  if (pos_lists_builder.Size() > 0) {
    output_tile->SetPositionListsAndVisibility(pos_lists_builder.Release());
    SetOutput(output_tile.release());
    return true;
  }
  // Try again
  else {
    // If we are out of any more pairs of child tiles to examine,
    // then we will return false earlier in this function
    // So, no need to return false here
   DExecute();
  }

  return true;
}

/**
 * @brief Advance the row iterator until value changes in terms of the join clauses
 * @return the end row number, [start_row, end_row) are the rows of the same value
 *         if the end_row == start_row, the subset is empty
 */
size_t MergeJoinExecutor::Advance(LogicalTile *tile, size_t start_row,
                                  bool is_left) {
  size_t end_row = start_row + 1;
  size_t this_row = start_row;
  size_t tuple_count = tile->GetTupleCount();
  if (start_row >= tuple_count)
    return start_row;

  while (end_row < tuple_count) {
    expression::ContainerTuple<executor::LogicalTile> this_tuple(tile,
                                                                 this_row);
    expression::ContainerTuple<executor::LogicalTile> next_tuple(tile, end_row);

    bool diff = false;

    for (auto &clause : *join_clauses_) {
      // Go through each join clauses
      auto expr = is_left ? clause.left_.get() : clause.right_.get();
      peloton::Value this_value = expr->Evaluate(&this_tuple, &this_tuple,
                                                 nullptr);
      peloton::Value next_value = expr->Evaluate(&next_tuple, &next_tuple,
                                                 nullptr);
      if (this_value.Compare(next_value) != 0) {
        diff = true;
        break;
      }
    }

    if (diff) {
      break;
    }

    // the two tuples are the same, we advance by 1
    end_row++;
    this_row = end_row;
  }

  LOG_TRACE("Advanced %s with subset size %lu", is_left ? "left" : "right",
      end_row - start_row);
  return end_row;
}

}  // namespace executor
}  // namespace peloton
