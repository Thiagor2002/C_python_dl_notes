template <bool> struct compile_time_error;
template <>     struct compile_time_error<true> {};

#define STATIC_ASSERT(_Expr, _Msg)                              \
    {                                                           \
        compile_time_error<((_Expr) != 0)> ERROR_##_Msg;        \
        (void)ERROR_##_Msg;                                     \
    }

int main()
{
    STATIC_ASSERT(sizeof(void*) == 4, requires_32_bit_platform);
}
