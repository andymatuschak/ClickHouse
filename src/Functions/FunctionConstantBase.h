#pragma once
#include <Functions/IFunction.h>
#include <Functions/FunctionFactory.h>
#include <Interpreters/Context.h>


namespace DB
{

/// Base class for constant functions
template<typename Derived, typename T, typename ColumnT>
class FunctionConstantBase : public IFunction
{
public:

    /// For server-level constants (uptime(), version(), etc)
    explicit FunctionConstantBase(ContextPtr context, T && constant_value_)
        : is_distributed(context->isDistributed())
        , constant_value(std::forward<T>(constant_value_))
    {
    }

    /// For real constants (pi(), e(), etc)
    explicit FunctionConstantBase(const T & constant_value_)
        : is_distributed(false)
        , constant_value(constant_value_)
    {
    }

    String getName() const override
    {
        return Derived::name;
    }

    size_t getNumberOfArguments() const override
    {
        return 0;
    }

    DataTypePtr getReturnTypeImpl(const DataTypes & /*arguments*/) const override
    {
        return std::make_shared<ColumnT>();
    }

    bool isDeterministic() const override { return false; }
    bool isDeterministicInScopeOfQuery() const override { return true; }

    /// Some functions may return different values on different shards/replicas, so it's not constant for distributed query
    bool isSuitableForConstantFolding() const override { return !is_distributed; }

    bool isSuitableForShortCircuitArgumentsExecution(const DataTypesWithConstInfo & /*arguments*/) const override { return false; }

    ColumnPtr executeImpl(const ColumnsWithTypeAndName &, const DataTypePtr &, size_t input_rows_count) const override
    {
        return ColumnT().createColumnConst(input_rows_count, constant_value);
    }

private:
    bool is_distributed;
    const T constant_value;
};

}

