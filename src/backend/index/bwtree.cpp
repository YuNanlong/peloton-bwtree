//===----------------------------------------------------------------------===//
//
//                         PelotonDB
//
// bwtree.cpp
//
// Identification: src/backend/index/bwtree.cpp
//
// Copyright (c) 2015, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <stack>

#include "backend/index/bwtree.h"
#include "backend/index/index_key.h"

namespace peloton {
namespace index {

template <typename KeyType, typename ValueType, class KeyComparator, class KeyEqualityChecker>
BWTree<KeyType, ValueType, KeyComparator, KeyEqualityChecker>::BWTree(
    const KeyComparator& comparator,
    const KeyEqualityChecker& equals) 
      : comparator_(comparator), equals_(equals) {
  root_ = 0;
  PID_counter_ = 1;
  allow_duplicate_ = true;

  InnerNode* root_base_page = new InnerNode();
  map_table_[root_] = root_base_page;

  // Can't do this here because we're using atomic inside vector
  // map_table_.resize(1000000);
}

// TODO (dana): can't get this to compile after I add the allow_duplicate param
// template <typename KeyType, typename ValueType, class KeyComparator, class KeyEqualityChecker>
// BWTree<KeyType, ValueType, KeyComparator, KeyEqualityChecker>::BWTree(
//     const KeyComparator& comparator,
//     const KeyEqualityChecker& equals,
//     bool allow_duplicate) 
//       : comparator_(comparator), equals_(equals), allow_duplicate_(allow_duplicate) {
//   root_ = 0;
//   PID_counter_ = 1;
//   allow_duplicate_ = allow_duplicate;
// 
//   InnerNode* root_base_page = new InnerNode();
//   map_table_[root_] = root_base_page;
// 
// }

template <typename KeyType, typename ValueType, class KeyComparator, class KeyEqualityChecker>
bool BWTree<KeyType, ValueType, KeyComparator, KeyEqualityChecker>::Insert(const KeyType& key,
                                                                           const ValueType& data) {
  while (true) {
    Page* root_page = map_table_[root_];
    if (root_page->GetType() == INNER_NODE &&
        reinterpret_cast<InnerNode*>(root_page)->children_.size() == 0) {
      // InnerNode* root_base_page = reinterpret_cast<InnerNode*>(root_page);

      // Construct our first leaf node
      LeafNode* leaf_base_page = new LeafNode();
      std::vector<ItemPointer> locations;
      locations.push_back(data);
      leaf_base_page->data_items_.push_back(std::make_pair(key, locations));
      leaf_base_page->low_key_ = key;
      leaf_base_page->high_key_ = key;
      leaf_base_page->absolute_min_ = true;
      leaf_base_page->absolute_max_ = true;

      // Install the leaf node to mapping table
      PID leaf_PID = InstallNewMapping(leaf_base_page);

      // Construct the index term delta record
      IndexTermDelta* index_term_page = new IndexTermDelta(key, key, leaf_PID);
      index_term_page->absolute_max_ = true;
      index_term_page->absolute_min_ = true;
      index_term_page->SetDeltaNext(root_page);

      // If prepending the IndexTermDelta fails, we need to free the resource
      // and start over the insert again.
      if (map_table_[root_].compare_exchange_strong(root_page,
                                                    index_term_page)) {
        return true;
      } else {
        // TODO: Garbage collect leaf_base_page, index_term_page and leaf_PID.

        continue;
      }
    } else {
      Page* current_page = root_page;
      PID current_PID = root_;
      //std::stack<PID> pages_visited; // To remember parent nodes traversed on our search path
      bool attempt_insert = true;
      while (attempt_insert) {
        switch (current_page->GetType()) {
          case INNER_NODE: {
            break;
          }
          case INDEX_TERM_DELTA: {
            IndexTermDelta* idx_delta = reinterpret_cast<IndexTermDelta*>(current_page);
            // If this key is > low_separator_ and <= high_separator_ then this is the child we
            // must visit next
            if ((idx_delta->absolute_min_ || comparator_(key, idx_delta->low_separator_) > 0) &&
                (idx_delta->absolute_max_ || comparator_(key, idx_delta->high_separator_) <= 0)) {
              // Follow the side link to this child
              current_PID = idx_delta->side_link_;
              current_page = map_table_[current_PID];
            } else {
              // This key does not fall within the boundary represented by this index term
              // delta record. Keep traversing the delta chain.
              current_page = current_page->GetDeltaNext(); 
            }
            continue;
          }
          case SPLIT_DELTA: {
            // TODO (dana)
            break;
          }
          case REMOVE_NODE_DELTA: {
            // TODO (dana)
            break;
          }
          case NODE_MERGE_DELTA: {
            // TODO (dana)
            break;
          }
          case LEAF_NODE: {
            __attribute__((unused)) LeafNode* leaf = reinterpret_cast<LeafNode*>(current_page);
            // Do binary search on data_items_ to see if key is in list
            // It's ok if it's in the tree already if we allow duplicates, otherwise return false
            break;
          }
          case MODIFY_DELTA: {
            __attribute__((unused)) ModifyDelta* mod_delta =
                reinterpret_cast<ModifyDelta*>(current_page);
            bool inserted;        // Whether this <key, value> pair is inserted 
            bool install_new;     // Whether we need to install a new MODIFY_DELTA
            if (equals_(key, mod_delta->key_)) {
              if (mod_delta->locations_.size() == 0) {
                // If the locations_ list is empty then we can insert regardless of the duplicate
                // keys policy (this was a delete operation)
                inserted = true;
                install_new = true;
              } else if (!allow_duplicate_) {
                // Key already exists in tree and duplicates are not allowed
                inserted = false;
              } else {
                auto found = std::find_if(mod_delta->locations_.begin(),
                    mod_delta->locations_.end(), [] (__attribute__((unused)) const ValueType& value) { 
                  // TODO: use ItemComparator here
                  return false;
                });
                if (found != mod_delta->locations_.end()) {
                  // TODO: if we find the exact same <key, value> pair when duplicates are enabled,
                  // does the insert fail? Or does it "succeed" but we don't have to do anything?
                  inserted = true;
                  install_new = false;
                } else {
                  // We can insert this <key, value> pair because duplicates are enabled and this
                  // value does not exist in locations_
                  inserted = true;
                  install_new = true;
                }
              }
            } else {
              // This is not our key so we keep traversing the delta chain
              current_page = current_page->GetDeltaNext(); 
              continue;
            }
            if (inserted && install_new) {
              // Install new ModifyDelta page
              // Copy old locations_ and add our new value
              std::vector<ValueType> locations(mod_delta->locations_);
              locations.push_back(data);
              ModifyDelta* new_modify_delta = new ModifyDelta(key, locations);
              new_modify_delta->SetDeltaNext(current_page);

              // If prepending the IndexTermDelta fails, we need to free the resource
              // and start over the insert again.
              if (map_table_[current_PID].compare_exchange_strong(current_page,
                                                                  new_modify_delta)) {
                return true;
              } else {
                // TODO: Garbage collect new_modify_delta.

                attempt_insert = false; // Start over
              }
            }
            return inserted;
          }
          default:
            throw IndexException("Unrecognized page type\n");
            break;
        }
      }
    }
  }

  return false;
}

// Explicit template instantiation
template class BWTree<IntsKey<1>, ItemPointer, IntsComparator<1>,
IntsEqualityChecker<1> >;
template class BWTree<IntsKey<2>, ItemPointer, IntsComparator<2>,
IntsEqualityChecker<2> >;
template class BWTree<IntsKey<3>, ItemPointer, IntsComparator<3>,
IntsEqualityChecker<3> >;
template class BWTree<IntsKey<4>, ItemPointer, IntsComparator<4>,
IntsEqualityChecker<4> >;

template class BWTree<GenericKey<4>, ItemPointer, GenericComparator<4>,
GenericEqualityChecker<4> >;
template class BWTree<GenericKey<8>, ItemPointer, GenericComparator<8>,
GenericEqualityChecker<8> >;
template class BWTree<GenericKey<12>, ItemPointer, GenericComparator<12>,
GenericEqualityChecker<12> >;
template class BWTree<GenericKey<16>, ItemPointer, GenericComparator<16>,
GenericEqualityChecker<16> >;
template class BWTree<GenericKey<24>, ItemPointer, GenericComparator<24>,
GenericEqualityChecker<24> >;
template class BWTree<GenericKey<32>, ItemPointer, GenericComparator<32>,
GenericEqualityChecker<32> >;
template class BWTree<GenericKey<48>, ItemPointer, GenericComparator<48>,
GenericEqualityChecker<48> >;
template class BWTree<GenericKey<64>, ItemPointer, GenericComparator<64>,
GenericEqualityChecker<64> >;
template class BWTree<GenericKey<96>, ItemPointer, GenericComparator<96>,
GenericEqualityChecker<96> >;
template class BWTree<GenericKey<128>, ItemPointer, GenericComparator<128>,
GenericEqualityChecker<128> >;
template class BWTree<GenericKey<256>, ItemPointer, GenericComparator<256>,
GenericEqualityChecker<256> >;
template class BWTree<GenericKey<512>, ItemPointer, GenericComparator<512>,
GenericEqualityChecker<512> >;

template class BWTree<TupleKey, ItemPointer, TupleKeyComparator,
TupleKeyEqualityChecker>;

}  // End index namespace
}  // End peloton namespace
