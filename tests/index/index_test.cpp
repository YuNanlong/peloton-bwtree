//===----------------------------------------------------------------------===//
//
//                         PelotonDB
//
// index_test.cpp
//
// Identification: tests/index/index_test.cpp
//
// Copyright (c) 2015, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "gtest/gtest.h"
#include "harness.h"

#include "backend/common/logger.h"
#include "backend/index/index_factory.h"
#include "backend/storage/tuple.h"

namespace peloton {
namespace test {

//===--------------------------------------------------------------------===//
// Index Tests
//===--------------------------------------------------------------------===//

catalog::Schema *key_schema = nullptr;
catalog::Schema *tuple_schema = nullptr;

ItemPointer item0(120, 5);
ItemPointer item1(120, 7);
ItemPointer item2(123, 19);

index::Index *BuildIndex() {
  // Build tuple and key schema
  std::vector<std::vector<std::string>> column_names;
  std::vector<catalog::Column> columns;
  std::vector<catalog::Schema *> schemas;
  IndexType index_type = INDEX_TYPE_BTREE;
  // TODO: Uncomment the line below
  index_type = INDEX_TYPE_BWTREE;

  catalog::Column column1(VALUE_TYPE_INTEGER, GetTypeSize(VALUE_TYPE_INTEGER),
                          "A", true);
  catalog::Column column2(VALUE_TYPE_VARCHAR, 1024, "B", true);
  catalog::Column column3(VALUE_TYPE_DOUBLE, GetTypeSize(VALUE_TYPE_DOUBLE),
                          "C", true);
  catalog::Column column4(VALUE_TYPE_INTEGER, GetTypeSize(VALUE_TYPE_INTEGER),
                          "D", true);

  columns.push_back(column1);
  columns.push_back(column2);

  // INDEX KEY SCHEMA -- {column1, column2}
  key_schema = new catalog::Schema(columns);
  key_schema->SetIndexedColumns({0, 1});

  columns.push_back(column3);
  columns.push_back(column4);

  // TABLE SCHEMA -- {column1, column2, column3, column4}
  tuple_schema = new catalog::Schema(columns);

  // Build index metadata
  const bool unique_keys = false;

  index::IndexMetadata *index_metadata = new index::IndexMetadata(
      "test_index", 125, index_type, INDEX_CONSTRAINT_TYPE_DEFAULT,
      tuple_schema, key_schema, unique_keys);

  // Build index
  index::Index *index = index::IndexFactory::GetInstance(index_metadata);
  EXPECT_TRUE(index != NULL);

  return index;
}

TEST(IndexTests, BasicTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex());

  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));

  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);

  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);

  // INSERT
  index->InsertEntry(key0.get(), item0);

  locations = index->ScanKey(key0.get());
  EXPECT_EQ(locations.size(), 1);
  EXPECT_EQ(locations[0].block, item0.block);

  // DELETE
  index->DeleteEntry(key0.get(), item0);

  locations = index->ScanKey(key0.get());
  EXPECT_EQ(locations.size(), 0);

  delete tuple_schema;
}

// INSERT HELPER FUNCTION
void InsertTest(index::Index *index, VarlenPool *pool, size_t scale_factor) {
  // Loop based on scale factor
  for (size_t scale_itr = 1; scale_itr <= scale_factor; scale_itr++) {
    // Insert a bunch of keys based on scale itr
    std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key3(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key4(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> keynonce(
        new storage::Tuple(key_schema, true));

    key0->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
    key1->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
    key2->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);
    key3->SetValue(0, ValueFactory::GetIntegerValue(400 * scale_itr), pool);
    key3->SetValue(1, ValueFactory::GetStringValue("d"), pool);
    key4->SetValue(0, ValueFactory::GetIntegerValue(500 * scale_itr), pool);
    key4->SetValue(1, ValueFactory::GetStringValue(
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"),
                   pool);
    keynonce->SetValue(0, ValueFactory::GetIntegerValue(1000 * scale_itr),
                       pool);
    keynonce->SetValue(1, ValueFactory::GetStringValue("f"), pool);

    // INSERT
    index->InsertEntry(key0.get(), item0);
    index->InsertEntry(key1.get(), item1);
    index->InsertEntry(key1.get(), item2);
    index->InsertEntry(key1.get(), item1);
    index->InsertEntry(key1.get(), item1);
    index->InsertEntry(key1.get(), item0);

    index->InsertEntry(key2.get(), item1);
    index->InsertEntry(key3.get(), item1);
    index->InsertEntry(key4.get(), item1);
  }

  LOG_DEBUG("Finish a lunch!");
}

// DELETE HELPER FUNCTION
void DeleteTest(index::Index *index, VarlenPool *pool, size_t scale_factor) {
  // Loop based on scale factor
  for (size_t scale_itr = 1; scale_itr <= scale_factor; scale_itr++) {
    // Delete a bunch of keys based on scale itr
    std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key3(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key4(new storage::Tuple(key_schema, true));

    key0->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
    key1->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
    key2->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);
    key3->SetValue(0, ValueFactory::GetIntegerValue(400 * scale_itr), pool);
    key3->SetValue(1, ValueFactory::GetStringValue("d"), pool);
    key4->SetValue(0, ValueFactory::GetIntegerValue(500 * scale_itr), pool);
    key4->SetValue(1, ValueFactory::GetStringValue(
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"),
                   pool);

    // DELETE
    index->DeleteEntry(key0.get(), item0);
    index->DeleteEntry(key1.get(), item1);
    index->DeleteEntry(key2.get(), item2);
    index->DeleteEntry(key3.get(), item1);
    index->DeleteEntry(key4.get(), item1);
  }
}

// INSERT HELPER FUNCTION
void HybridTestP1(index::Index *index, VarlenPool *pool, size_t scale_factor) {
  // Loop based on scale factor
  for (size_t scale_itr = 1; scale_itr <= scale_factor; scale_itr++) {
    // Insert a bunch of keys based on scale itr
    std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key3(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key4(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> keynonce(
        new storage::Tuple(key_schema, true));

    key0->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
    key1->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
    key2->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);
    key3->SetValue(0, ValueFactory::GetIntegerValue(400 * scale_itr), pool);
    key3->SetValue(1, ValueFactory::GetStringValue("d"), pool);
    key4->SetValue(0, ValueFactory::GetIntegerValue(500 * scale_itr), pool);
    key4->SetValue(1, ValueFactory::GetStringValue(
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"),
                   pool);
    keynonce->SetValue(0, ValueFactory::GetIntegerValue(1000 * scale_itr),
                       pool);
    keynonce->SetValue(1, ValueFactory::GetStringValue("f"), pool);

    // INSERT
    index->InsertEntry(key0.get(), item0);
    index->InsertEntry(key1.get(), item1);

    index->DeleteEntry(key0.get(), item0);
    index->DeleteEntry(key1.get(), item1);
  }
}

// INSERT HELPER FUNCTION
void HybridTestP2(index::Index *index, VarlenPool *pool, size_t scale_factor) {
  // Loop based on scale factor
  for (size_t scale_itr = 1; scale_itr <= scale_factor; scale_itr++) {
    // Insert a bunch of keys based on scale itr
    std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key3(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key4(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> keynonce(
        new storage::Tuple(key_schema, true));

    key0->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
    key1->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
    key2->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);
    key3->SetValue(0, ValueFactory::GetIntegerValue(400 * scale_itr), pool);
    key3->SetValue(1, ValueFactory::GetStringValue("d"), pool);
    key4->SetValue(0, ValueFactory::GetIntegerValue(500 * scale_itr), pool);
    key4->SetValue(1, ValueFactory::GetStringValue(
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"),
                   pool);
    keynonce->SetValue(0, ValueFactory::GetIntegerValue(1000 * scale_itr),
                       pool);
    keynonce->SetValue(1, ValueFactory::GetStringValue("f"), pool);

    index->InsertEntry(key1.get(), item2);
    index->InsertEntry(key1.get(), item0);

    index->InsertEntry(key2.get(), item1);
    index->InsertEntry(key3.get(), item1);
    index->InsertEntry(key4.get(), item1);
  }
}

// INSERT HELPER FUNCTION
void HybridTestP3(index::Index *index, VarlenPool *pool, size_t scale_factor) {
  // Loop based on scale factor
  for (size_t scale_itr = 1; scale_itr <= scale_factor; scale_itr++) {
    // Insert a bunch of keys based on scale itr
    std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key3(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key4(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> keynonce(
        new storage::Tuple(key_schema, true));

    key0->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
    key1->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
    key2->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);
    key3->SetValue(0, ValueFactory::GetIntegerValue(400 * scale_itr), pool);
    key3->SetValue(1, ValueFactory::GetStringValue("d"), pool);
    key4->SetValue(0, ValueFactory::GetIntegerValue(500 * scale_itr), pool);
    key4->SetValue(1, ValueFactory::GetStringValue(
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"),
                   pool);
    keynonce->SetValue(0, ValueFactory::GetIntegerValue(1000 * scale_itr),
                       pool);
    keynonce->SetValue(1, ValueFactory::GetStringValue("f"), pool);

    index->DeleteEntry(key4.get(), item1);
  }
}

// INSERT HELPER FUNCTION
void coolTest(index::Index *index, VarlenPool *pool, size_t scale_factor) {
  // Loop based on scale factor
  for (size_t scale_itr = 1; scale_itr <= scale_factor; scale_itr++) {
    // Insert a bunch of keys based on scale itr
    std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key3(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key4(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> keynonce(
        new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key5(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key6(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key7(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key8(new storage::Tuple(key_schema, true));
    std::unique_ptr<storage::Tuple> key9(new storage::Tuple(key_schema, true));

    key0->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
    key1->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
    key2->SetValue(0, ValueFactory::GetIntegerValue(100 * scale_itr), pool);
    key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);
    key3->SetValue(0, ValueFactory::GetIntegerValue(400 * scale_itr), pool);
    key3->SetValue(1, ValueFactory::GetStringValue("d"), pool);
    key4->SetValue(0, ValueFactory::GetIntegerValue(500 * scale_itr), pool);
    key4->SetValue(1, ValueFactory::GetStringValue(
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                          "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"),
                   pool);
    keynonce->SetValue(0, ValueFactory::GetIntegerValue(1000 * scale_itr),
                       pool);
    keynonce->SetValue(1, ValueFactory::GetStringValue("f"), pool);

    key5->SetValue(0, ValueFactory::GetIntegerValue(250), pool);
    key5->SetValue(1, ValueFactory::GetStringValue("hello"), pool);
    key6->SetValue(0, ValueFactory::GetIntegerValue(10), pool);
    key6->SetValue(1, ValueFactory::GetStringValue("goodbye"), pool);
    key7->SetValue(0, ValueFactory::GetIntegerValue(5000), pool);
    key7->SetValue(1, ValueFactory::GetStringValue("something"), pool);
    key8->SetValue(0, ValueFactory::GetIntegerValue(89), pool);
    key8->SetValue(1, ValueFactory::GetStringValue("dad"), pool);
    key9->SetValue(0, ValueFactory::GetIntegerValue(4020), pool);
    key9->SetValue(1, ValueFactory::GetStringValue("mom"), pool);

    index->InsertEntry(key5.get(), item0);
    index->InsertEntry(key6.get(), item1);
    index->InsertEntry(key7.get(), item2);
    index->InsertEntry(key8.get(), item1);
    index->InsertEntry(key9.get(), item2);
  }
}

TEST(IndexTests, DeleteTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex());

  // Single threaded test
  size_t scale_factor = 1;
  LaunchParallelTest(1, InsertTest, index.get(), pool, scale_factor);
  LaunchParallelTest(1, DeleteTest, index.get(), pool, scale_factor);

  // Checks
  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));

  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
  key1->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
  key2->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);

  locations = index->ScanKey(key0.get());
  EXPECT_EQ(locations.size(), 0);

  locations = index->ScanKey(key1.get());
  EXPECT_EQ(locations.size(), 2);

  locations = index->ScanKey(key2.get());
  EXPECT_EQ(locations.size(), 1);
  EXPECT_EQ(locations[0].block, item1.block);

  delete tuple_schema;
}

TEST(IndexTests, ScanTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex());

  // Single threaded test
  size_t scale_factor = 2;
  LaunchParallelTest(1, InsertTest, index.get(), pool, scale_factor);

  std::vector<oid_t> key_column_ids;

  std::vector<ExpressionType> expr_types;

  std::vector<peloton::Value> values;

  // constrain the search on key1 and key2
  key_column_ids.push_back(0);
  expr_types.push_back(EXPRESSION_TYPE_COMPARE_EQUAL);
  values.push_back(ValueFactory::GetIntegerValue(100));
  key_column_ids.push_back(1);
  expr_types.push_back(EXPRESSION_TYPE_COMPARE_GREATERTHAN);
  values.push_back(ValueFactory::GetStringValue("a"));
  key_column_ids.push_back(1);
  expr_types.push_back(EXPRESSION_TYPE_COMPARE_LESSTHAN);
  values.push_back(ValueFactory::GetStringValue("d"));

  // Check scan
  locations = index->Scan(values, key_column_ids, expr_types,
                                   SCAN_DIRECTION_TYPE_FORWARD);
  EXPECT_EQ(locations.size(), 6);

  LaunchParallelTest(1, DeleteTest, index.get(), pool, scale_factor);
  locations = index->Scan(values, key_column_ids, expr_types,
                                   SCAN_DIRECTION_TYPE_FORWARD);
  EXPECT_EQ(locations.size(), 3);
  EXPECT_EQ(locations[0].block, item2.block);

  delete tuple_schema;
}

TEST(IndexTests, MultiThreadedInsertTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex());

  // Parallel Test
  size_t num_threads = 4;
  size_t scale_factor = 2;
  LaunchParallelTest(num_threads, InsertTest, index.get(), pool, scale_factor);

  locations = index->ScanAllKeys();
  EXPECT_EQ(locations.size(), 9 * num_threads * scale_factor);

  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> keynonce(
      new storage::Tuple(key_schema, true));

  keynonce->SetValue(0, ValueFactory::GetIntegerValue(1000), pool);
  keynonce->SetValue(1, ValueFactory::GetStringValue("f"), pool);

  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);

  locations = index->ScanKey(keynonce.get());
  EXPECT_EQ(locations.size(), 0);

  locations = index->ScanKey(key0.get());
  EXPECT_EQ(locations.size(), num_threads);
  EXPECT_EQ(locations[0].block, item0.block);

  delete tuple_schema;
}

TEST(IndexTests, MultiThreadedInsertMoreTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex());

  // Parallel Test
  size_t num_threads = 100;
  size_t scale_factor = 2;
  LaunchParallelTest(num_threads, InsertTest, index.get(), pool, scale_factor);

  LOG_DEBUG("here1");
  locations = index->ScanAllKeys();
  LOG_DEBUG("here2");
  EXPECT_EQ(locations.size(), 9 * num_threads * scale_factor);

  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key3(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key4(new storage::Tuple(key_schema, true));

  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
  key1->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
  key2->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);
  key3->SetValue(0, ValueFactory::GetIntegerValue(400), pool);
  key3->SetValue(1, ValueFactory::GetStringValue("d"), pool);
  key4->SetValue(0, ValueFactory::GetIntegerValue(500), pool);
  key4->SetValue(1, ValueFactory::GetStringValue(
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"),
                 pool);

  std::unique_ptr<storage::Tuple> keynonce(
      new storage::Tuple(key_schema, true));

  keynonce->SetValue(0, ValueFactory::GetIntegerValue(1000), pool);
  keynonce->SetValue(1, ValueFactory::GetStringValue("f"), pool);

  LOG_DEBUG("here scan key");
  locations = index->ScanKey(keynonce.get());
  EXPECT_EQ(locations.size(), 0);

  LOG_DEBUG("here scan key");
  locations = index->ScanKey(key0.get());
  EXPECT_EQ(locations.size(), num_threads);
  EXPECT_EQ(locations[0].block, item0.block);
  LOG_DEBUG("here scan key");
  locations = index->ScanKey(key1.get());
  EXPECT_EQ(locations.size(), num_threads * 5);
  EXPECT_EQ(locations[0].block, item1.block);
  LOG_DEBUG("here scan key");
  locations = index->ScanKey(key2.get());
  EXPECT_EQ(locations.size(), num_threads);
  EXPECT_EQ(locations[0].block, item1.block);
  LOG_DEBUG("here scan key");
  locations = index->ScanKey(key3.get());
  EXPECT_EQ(locations.size(), num_threads);
  EXPECT_EQ(locations[0].block, item1.block);
  LOG_DEBUG("here scan key");
  locations = index->ScanKey(key4.get());
  EXPECT_EQ(locations.size(), num_threads);
  EXPECT_EQ(locations[0].block, item1.block);

  delete tuple_schema;
  LOG_DEBUG("Finish multi-thread more insert test!");
}

TEST(IndexTests, MultiThreadedHybirdMoreTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex());

  locations = index->ScanAllKeys();
  EXPECT_EQ(locations.size(), 0);

  // Parallel Test
  size_t num_threads = 100;
  size_t scale_factor = 1;
  LaunchParallelTest(num_threads, HybridTestP1, index.get(), pool, scale_factor);
  LaunchParallelTest(num_threads, HybridTestP2, index.get(), pool, scale_factor);
  LaunchParallelTest(num_threads, HybridTestP3, index.get(), pool, scale_factor);

  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key3(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key4(new storage::Tuple(key_schema, true));

  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
  key1->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
  key2->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);
  key3->SetValue(0, ValueFactory::GetIntegerValue(400), pool);
  key3->SetValue(1, ValueFactory::GetStringValue("d"), pool);
  key4->SetValue(0, ValueFactory::GetIntegerValue(500), pool);
  key4->SetValue(1, ValueFactory::GetStringValue(
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"),
                 pool);

  std::unique_ptr<storage::Tuple> keynonce(
      new storage::Tuple(key_schema, true));

  keynonce->SetValue(0, ValueFactory::GetIntegerValue(1000), pool);
  keynonce->SetValue(1, ValueFactory::GetStringValue("f"), pool);

  locations = index->ScanKey(key0.get());
  EXPECT_EQ(locations.size(), 0);

  locations = index->ScanKey(key1.get());
  EXPECT_EQ(locations.size(), num_threads * 2);

  locations = index->ScanKey(key2.get());
  EXPECT_EQ(locations.size(), num_threads);

  locations = index->ScanKey(key3.get());
  EXPECT_EQ(locations.size(), num_threads);

  locations = index->ScanKey(key4.get());
  EXPECT_EQ(locations.size(), 0);

  delete tuple_schema;
}

TEST(IndexTests, AnotherDamnTest) {
  auto pool = TestingHarness::GetInstance().GetTestingPool();
  std::vector<ItemPointer> locations;

  // INDEX
  std::unique_ptr<index::Index> index(BuildIndex());

  locations = index->ScanAllKeys();
  EXPECT_EQ(locations.size(), 0);

  // Parallel Test
  size_t num_threads = 100;
  size_t scale_factor = 1;
  LaunchParallelTest(num_threads, InsertTest, index.get(), pool, scale_factor);
  LaunchParallelTest(num_threads, coolTest, index.get(), pool, scale_factor);

  std::unique_ptr<storage::Tuple> key0(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key1(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key2(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key3(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key4(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key5(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key6(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key7(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key8(new storage::Tuple(key_schema, true));
  std::unique_ptr<storage::Tuple> key9(new storage::Tuple(key_schema, true));

  key0->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key0->SetValue(1, ValueFactory::GetStringValue("a"), pool);
  key1->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key1->SetValue(1, ValueFactory::GetStringValue("b"), pool);
  key2->SetValue(0, ValueFactory::GetIntegerValue(100), pool);
  key2->SetValue(1, ValueFactory::GetStringValue("c"), pool);
  key3->SetValue(0, ValueFactory::GetIntegerValue(400), pool);
  key3->SetValue(1, ValueFactory::GetStringValue("d"), pool);
  key4->SetValue(0, ValueFactory::GetIntegerValue(500), pool);
  key4->SetValue(1, ValueFactory::GetStringValue(
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
                        "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"),
                 pool);

  std::unique_ptr<storage::Tuple> keynonce(
      new storage::Tuple(key_schema, true));

  keynonce->SetValue(0, ValueFactory::GetIntegerValue(1000), pool);
  keynonce->SetValue(1, ValueFactory::GetStringValue("f"), pool);

  key5->SetValue(0, ValueFactory::GetIntegerValue(250), pool);
  key5->SetValue(1, ValueFactory::GetStringValue("hello"), pool);
  key6->SetValue(0, ValueFactory::GetIntegerValue(10), pool);
  key6->SetValue(1, ValueFactory::GetStringValue("goodbye"), pool);
  key7->SetValue(0, ValueFactory::GetIntegerValue(5000), pool);
  key7->SetValue(1, ValueFactory::GetStringValue("something"), pool);
  key8->SetValue(0, ValueFactory::GetIntegerValue(89), pool);
  key8->SetValue(1, ValueFactory::GetStringValue("dad"), pool);
  key9->SetValue(0, ValueFactory::GetIntegerValue(4020), pool);
  key9->SetValue(1, ValueFactory::GetStringValue("mom"), pool);

  locations = index->ScanKey(key0.get());
  EXPECT_EQ(locations.size(), num_threads);
  EXPECT_EQ(locations[0].block, item0.block);

  locations = index->ScanKey(key5.get());
  EXPECT_EQ(locations.size(), num_threads);

  locations = index->ScanKey(key6.get());
  EXPECT_EQ(locations.size(), num_threads);

  locations = index->ScanKey(key7.get());
  EXPECT_EQ(locations.size(), num_threads);

  locations = index->ScanKey(key8.get());
  EXPECT_EQ(locations.size(), num_threads);

  locations = index->ScanKey(key9.get());
  EXPECT_EQ(locations.size(), num_threads);

  delete tuple_schema;
  LOG_DEBUG("Finish another damn test!");
}

}  // End test namespace
}  // End peloton namespace
