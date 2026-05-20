#ifndef TYPE_DISPLAY

#define TYPE_DISPLAY(var) \
    static type_displayer<decltype(var)> type_display_test

template <typename T>  // declaration only for type_displayer;
class type_displayer;

#endif // TYPE_DISPLAY
