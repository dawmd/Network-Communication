#ifndef __MOVE_ONLY_FUNCTION_H__
#define __MOVE_ONLY_FUNCTION_H__


#include <memory>       // std::unique_ptr
#include <type_traits>  // std::is_invocable_r_v

namespace Network {

template<typename T>
class MoveOnlyFunction;

template<typename R, typename... Args>
class MoveOnlyFunction<R(Args...)> {
private:
    struct BaseFunction {
        virtual ~BaseFunction() = default;
        virtual R operator()(Args ...args) = 0;
    };

    template<typename F>
        requires (std::is_invocable_r_v<R, F, Args...>)
    struct ModelFunction : BaseFunction {
        F function;

        ModelFunction(F &&f)
        : function{std::move(f)} {}

        R operator()(Args ...args) override {
            return function(args...);
        }
    };

private:
    std::unique_ptr<BaseFunction> function;

public:
    MoveOnlyFunction() = default;

    template<typename F>
        requires (std::is_invocable_r_v<R, F, Args...>)
    MoveOnlyFunction(F &&f)
    : function{std::make_unique<ModelFunction<F>>(std::move(f))} {}

    template<typename F>
        requires (std::is_invocable_r_v<R, F, Args...>)
    MoveOnlyFunction &operator=(F &&f) {
        function = std::make_unique<ModelFunction<F>>(std::move(f));
        return *this;
    }

    MoveOnlyFunction(MoveOnlyFunction&&) = default;
    MoveOnlyFunction &operator=(MoveOnlyFunction&&) = default;

    MoveOnlyFunction(const MoveOnlyFunction&) = delete;
    MoveOnlyFunction &operator=(const MoveOnlyFunction&) = delete;

    ~MoveOnlyFunction() = default;

    R operator()(Args ...args) {
        return function->operator()(args...);
    }
};

} // namespace Network

#endif // __MOVE_ONLY_FUNCTION_H__

