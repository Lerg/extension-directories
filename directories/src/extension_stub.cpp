#if !defined(DM_PLATFORM_ANDROID) && !defined(DM_PLATFORM_IOS) && !defined(DM_PLATFORM_OSX) && !defined(DM_PLATFORM_LINUX) && !defined(DM_PLATFORM_WINDOWS)

#include <dmsdk/sdk.h>
#include "extension.h"

typedef enum {
	system_directory_application_support,
	system_directory_caches,
	system_directory_documents,
	system_directory_resource,
	system_directory_temporary
}   SystemDirectory;

static int path_for_file(lua_State *L) {
	dmLogInfo(EXTENSION_NAME_STRING " extenstion is not supported on this platform.");
	const char* file_path = lua_tostring(L, 1);
	lua_pushstring(L, file_path);
	return 1;
}

dmExtension::Result APP_INITIALIZE(dmExtension::AppParams *params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result APP_FINALIZE(dmExtension::AppParams *params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result INITIALIZE(dmExtension::Params *params) {
	lua_State *L = params->m_L;
	const luaL_Reg lua_functions[] = {
		{"path_for_file", path_for_file},
		{NULL, NULL}
	};

	luaL_openlib(L, EXTENSION_NAME_STRING, lua_functions, 1);

	lua_pushnumber(L, system_directory_application_support);
	lua_setfield(L, -2, "application_support");

	lua_pushnumber(L, system_directory_caches);
	lua_setfield(L, -2, "caches");

	lua_pushnumber(L, system_directory_documents);
	lua_setfield(L, -2, "documents");

	lua_pushnumber(L, system_directory_resource);
	lua_setfield(L, -2, "resource");

	lua_pushnumber(L, system_directory_temporary);
	lua_setfield(L, -2, "temporary");

	return dmExtension::RESULT_OK;
}

dmExtension::Result UPDATE(dmExtension::Params *params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result FINALIZE(dmExtension::Params *params) {
	return dmExtension::RESULT_OK;
}

DECLARE_DEFOLD_EXTENSION

#endif
