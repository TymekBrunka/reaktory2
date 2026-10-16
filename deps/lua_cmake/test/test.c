#include <lualib.h>
#include <lauxlib.h>
#include <lua.h>

int main() {
  lua_State* L = luaL_newstate();
  luaL_openlibs(L); // load print etc
  luaL_dostring(L, "x = 42\nprint(\"x is \" .. x)");
  lua_close(L);
}
