#if defined(DM_PLATFORM_HTML5)

#include "extension.h"
#define DLIB_LOG_DOMAIN EXTENSION_NAME_STRING
#include <dmsdk/sdk.h>

#include <emscripten.h>
#include <libgen.h>
#include <limits.h>
#include <pwd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

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
	struct stat st = {0};
	if (stat(path, &st) == -1 && mkdir(path, 0755) != 0) {
		return false;
	}
	return true;
}

static int path_for_file(lua_State *L) {
	int count = lua_gettop(L);
	const char *file_path = NULL;
	if (!lua_isnil(L, 1)) {
		file_path = lua_tostring(L, 1);
	}

	char home_app_path[PATH_MAX];
	snprintf(home_app_path, PATH_MAX, "/data/.%s", app_id);

	if (!create_dir(home_app_path)) {
		dmLogError("Failed to create app directory in user's home: %s", home_app_path);
		return 0;
	}

	SystemDirectory directory = system_directory_resource;
	if (count == 2) {
		directory = (SystemDirectory)lua_tonumber(L, 2);
	}
	const char *directory_path = NULL;
	switch (directory) {
		case system_directory_application_support:
			directory_path = copy_string(home_app_path);
			break;
		case system_directory_caches: {
				char path[PATH_MAX];
				snprintf(path, PATH_MAX, "%s/Caches", home_app_path);
				directory_path = copy_string(path);
				if (!create_dir(directory_path)) {
					dmLogError("Failed to create Caches directory in app's directory: %s", directory_path);
					return 0;
				}
			}
			break;
		case system_directory_documents: {
				char path[PATH_MAX];
				snprintf(path, PATH_MAX, "%s/Documents", home_app_path);
				directory_path = copy_string(path);
				if (!create_dir(directory_path)) {
					dmLogError("Failed to create Documents directory in app's directory: %s", directory_path);
					return 0;
				}
			}
			break;
		case system_directory_resource:
			directory_path = copy_string("/");
			break;
		case system_directory_temporary: {
				char path[PATH_MAX];
				snprintf(path, PATH_MAX, "/tmp/%s", app_id);
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
		char path[PATH_MAX];
		snprintf(path, PATH_MAX, "%s/%s", directory_path, file_path);
		lua_pushstring(L, path);
	} else {
		lua_pushstring(L, directory_path);
	}
	delete []directory_path;
	return 1;
}

static int download_file(lua_State *L) {
	int count = lua_gettop(L);
	const char *filename = NULL;
	const char *download_name = NULL;
	if (lua_isstring(L, 1)) {
		filename = lua_tostring(L, 1);
	} else {
		dmLogError("filename is required.");
		return 0;
	}
	if (lua_isstring(L, 2)) {
		download_name = lua_tostring(L, 2);
	} else {
		download_name = basename((char *)filename);
	}
	EM_ASM_({
		var filename = Pointer_stringify($0);
		var download_name = Pointer_stringify($1);
		var file;
		var data = FS.readFile(filename).buffer;
		var properties = {type: 'application/octet-stream'};
		try {
			file = new File([data], download_name, properties);
		} catch (e) {
			file = new Blob([data], properties);
		}
		var url = URL.createObjectURL(file);
		var a = document.createElement('a');
		a.href = url;
		a.download = download_name;
		a.click();
	}, filename, download_name);
	return 0;
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
		{"download_file", download_file},
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
