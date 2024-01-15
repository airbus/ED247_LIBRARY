#define QUOTE(m) #m
#define STRING_MACRO(m) QUOTE(m)
const char* VERSION = "Version " STRING_MACRO(_PRODUCT_VERSION);
