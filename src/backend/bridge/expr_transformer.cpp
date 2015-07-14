/*-------------------------------------------------------------------------
 *
 * expr_transformer.cpp
 * file description
 *
 * Copyright(c) 2015, CMU
 *
 *-------------------------------------------------------------------------
 */

#include "nodes/pprint.h"
#include "utils/rel.h"
#include "utils/lsyscache.h"
#include "bridge/bridge.h"
#include "executor/executor.h"
#include "parser/parsetree.h"

#include "expr_transformer.h"


#include "backend/bridge/tuple_transformer.h"
#include "backend/common/logger.h"
#include "backend/common/value.h"
#include "backend/common/value_factory.h"
#include "backend/expression/expression_util.h"

namespace peloton {
namespace bridge {

/**
 * @brief Helper function: re-map Postgres's builtin function Oid
 * to proper expression type in Peloton
 *
 * @param pg_func_id  PG Function Id used to lookup function in \b fmrg_builtin[]
 * (see Postgres source file 'fmgrtab.cpp')
 * @param lc          Left child.
 * @param rc          Right child.
 * @return            Corresponding expression tree in peloton.
 */
expression::AbstractExpression*
ReMapPgFunc(Oid pg_func_id,
            expression::AbstractExpression* lc,
            expression::AbstractExpression* rc);

void ExprTransformer::PrintPostgressExprTree(const ExprState* expr_state, std::string prefix) {
  auto tag = nodeTag(expr_state->expr);

  // TODO Not complete.
  LOG_INFO("%u ", tag);
}

/**
 * @brief Transform a ExprState tree (Postgres) to a AbstractExpression tree (Peloton) recursively.
 */
expression::AbstractExpression* ExprTransformer::TransformExpr(
    const ExprState* expr_state) {

  if(!expr_state)
    return nullptr;

  expression::AbstractExpression* peloton_expr = nullptr;

  switch(nodeTag(expr_state->expr)){
    case T_Const:
      peloton_expr = TransformConstant(expr_state);
      break;

    case T_OpExpr:
      peloton_expr = TransformOp(expr_state);
      break;

    case T_Var:
      peloton_expr = TransformVar(expr_state);
      break;

    default:
      LOG_ERROR("Unsupported Postgres Expr type: %u\n", nodeTag(expr_state->expr));
  }




  return peloton_expr;
}

bool ExprTransformer::CleanExprTree(
    expression::AbstractExpression* root) {

  // AbstractExpression's destructor already handles deleting children
  delete root;

  return true;
}

expression::AbstractExpression* ExprTransformer::TransformConstant(
    const ExprState* es) {

  auto const_expr = reinterpret_cast<const Const*>(es->expr);

  if(!(const_expr->constbyval)) {
    LOG_ERROR("Sorry, we don't handle by-reference constant values currently.\n");
  }

  Value value;

  if(const_expr->constisnull){ // Constant is null
    value = ValueFactory::GetNullValue();
  }
  else {  // non null
    value = TupleTransformer::GetValue(const_expr->constvalue, const_expr->consttype);
  }

  LOG_INFO("Const : ");
  std::cout << value << std::endl;

  // A Const Expr has no children.
  return expression::ConstantValueFactory(value);
}


expression::AbstractExpression* ExprTransformer::TransformOp(
    const ExprState* es) {

  auto op_expr = reinterpret_cast<const OpExpr*>(es->expr);
  auto func_state = reinterpret_cast<const FuncExprState*>(es);

  assert(op_expr->opfuncid != 0); // Hopefully it has been filled in by PG planner
  assert(list_length(func_state->args) <= 2);   // Hopefully it has at most two parameters

  expression::AbstractExpression* lc = nullptr;
  expression::AbstractExpression* rc = nullptr;

  // Add function arguments as children
  int i = 0;
  ListCell   *arg;
  foreach(arg, func_state->args)
  {
    ExprState  *argstate = (ExprState *) lfirst(arg);

    if(i == 0)
      lc = TransformExpr(argstate);
    else if(i == 1)
      rc = TransformExpr(argstate);
    else
      break; // skip >2 arguments

    i++;
  }

  return ReMapPgFunc(op_expr->opfuncid, lc, rc);
}

expression::AbstractExpression* ExprTransformer::TransformVar(
    const ExprState* es) {

  // Var expr only needs default ES
  auto var_expr = reinterpret_cast<const Var*>(es->expr);

  assert(var_expr->varattno != InvalidAttrNumber);

  oid_t tuple_idx = (var_expr->varno == OUTER_VAR ? 1 : 0);  // Seems reasonable, c.f. ExecEvalScalarVarFast()
  oid_t value_idx = static_cast<oid_t>(var_expr->varattno - 1); // Damnit attno is 1-index

  LOG_INFO("tuple_idx = %u , value_idx = %u \n", tuple_idx, value_idx);

  // TupleValue expr has no children.
  return expression::TupleValueFactory(tuple_idx, value_idx);

}

expression::AbstractExpression*
ReMapPgFunc(Oid func_id,
            expression::AbstractExpression* lc,
            expression::AbstractExpression* rc) {

  /**
   * Read fmgrtab.cpp
   */
  switch(func_id) {

    case 63:
    case 65:
    case 67:
    case 158:
    case 159:
      return expression::ComparisonFactory(EXPRESSION_TYPE_COMPARE_EQ, lc, rc);

    case 84:
    case 144:
    case 145:
    case 157:
    case 164:
    case 165:
      return expression::ComparisonFactory(EXPRESSION_TYPE_COMPARE_NE, lc, rc);

    default:
      LOG_ERROR("Unsupported PG Function ID : %u (check fmgrtab.cpp)\n", func_id);
  }
  return nullptr;
}



} /* namespace bridge */
} /* namespace peloton */
