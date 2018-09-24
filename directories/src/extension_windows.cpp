#if defined(DM_PLATFORM_WINDOWS)

#include "extension.h"
#define DLIB_LOG_DOMAIN EXTENSION_NAME_STRING
#include <dmsdk/sdk.h>

#include <stdio.h>
#include <Windows.h>
#include <shlobj.h>
#include <shlwapi.h>

typedef enum {
	system_directory_application_support,
	system_directory_caches,
	system_directory_documents,
	system_directory_resource,
	system_directory_temporary
}   SystemDirectory;

static const char *app_id = NULL;

static char *copy_string(const char *source) {
	if (source != NULL) {
		size_t length = strlen(source) + 1;
		char *destination = new char[length];
		strncpy(destination, source, length);
		destination[length - 1] = 0;
		return destination;
	}
	return NULL;
}

static bool create_dir(const char *path) {
	if (CreateDirectory(path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
		return true;
	}
	return false;
}

static int path_for_file(lua_State *L) {
	int count = lua_gettop(L);
	const char *file_path = NULL;
	if (!lua_isnil(L, 1)) {
		file_path = lua_tostring(L, 1);
	}

	char app_data_roaming_path[MAX_PATH];
	if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, app_data_roaming_path))) {
		dmLogError("Failed to get AppData\\Roaming directory path.");
		return 0;
	}

	char app_data_roaming_app_path[MAX_PATH];
	snprintf(app_data_roaming_app_path, MAX_PATH, "%s\\%s", app_data_roaming_path, app_id);

	if (!create_dir(app_data_roaming_app_path)) {
		dmLogError("Failed to create app directory in AppData\\Roaming directory: %s", app_data_roaming_app_path);
		return 0;
	}

	SystemDirectory directory = system_directory_resource;
	if (count == 2) {
		directory = (SystemDirectory)((int)lua_tonumber(L, 2));
	}
	const char *directory_path = NULL;
	switch (directory) {
		case system_directory_application_support:
			directory_path = copy_string(app_data_roaming_app_path);
			break;
		case system_directory_caches: {
				char path[MAX_PATH];
				snprintf(path, MAX_PATH, "%s\\Caches", app_data_roaming_app_path);
				directory_path = copy_string(path);
				if (!create_dir(directory_path)) {
					dmLogError("Failed to create Caches directory in app's directory: %s", directory_path);
					return 0;
				}
			}
			break;
		case system_directory_documents: {
				char path[MAX_PATH];
				snprintf(path, MAX_PATH, "%s\\Documents", app_data_roaming_app_path);
				directory_path = copy_string(path);
				if (!create_dir(directory_path)) {
					dmLogError("Failed to create Documents directory in app's directory: %s", directory_path);
					return 0;
				}
			}
			break;
		case system_directory_resource: {
				char path[MAX_PATH];
				GetModuleFileName(NULL, path, MAX_PATH);
				PathRemoveFileSpec(path);
				directory_path = copy_string(path);
			}
			break;
		case system_directory_temporary: {
				char tmp_path[MAX_PATH];
				GetTempPath(MAX_PATH, tmp_path);
				char path[MAX_PATH];
				snprintf(path, MAX_PATH, "%s%s", tmp_path, app_id);
				directory_path = copy_string(path);
				if (!create_dir(directory_path)) {
					dmLogError("Failed to create temporary directory: %s", directory_path);
					return 0;
				}
			}
			break;
		default:
			dmLogError("Unsupported directory");
			return 0;
	}
	if (file_path) {
		char path[MAX_PATH];
		snprintf(path, MAX_PATH, "%s\\%s", directory_path, file_path);
		lua_pushstring(L, path);
	} else {
		lua_pushstring(L, directory_path);
	}
	delete []directory_path;
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

	const char *app_id_option = dmConfigFile::GetString(params->m_ConfigFile, "osx.bundle_identifier", NULL);
	if (app_id_option == NULL) {
		dmLogWarning("osx.bundle_identifier is not set, using project title instead.");
		app_id_option = dmConfigFile::GetString(params->m_ConfigFile, "project.title", "defold");
	}
	app_id = copy_string(app_id_option);

	return dmExtension::RESULT_OK;
}

dmExtension::Result UPDATE(dmExtension::Params *params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result FINALIZE(dmExtension::Params *params) {
	delete []app_id;
	return dmExtension::RESULT_OK;
}

DECLARE_DEFOLD_EXTENSION

#endif
