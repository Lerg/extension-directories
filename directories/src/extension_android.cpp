#if defined(DM_PLATFORM_ANDROID)

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
	ThreadAttacher attacher;
	JNIEnv *env = attacher.env;
	ClassLoader class_loader = ClassLoader(env);
	jclass string_class = class_loader.load("java/lang/String");
	jmethodID string_concat = env->GetMethodID(string_class, "concat", "(Ljava/lang/String;)Ljava/lang/String;");

	jclass file_class = class_loader.load("java/io/File");
	jmethodID file_constructor = env->GetMethodID(file_class, "<init>", "(Ljava/lang/String;)V");
	jmethodID file_exists = env->GetMethodID(file_class, "exists", "()Z");
	jmethodID file_mkdirs = env->GetMethodID(file_class, "mkdirs", "()Z");
	jmethodID file_get_canonical_path = env->GetMethodID(file_class, "getCanonicalPath", "()Ljava/lang/String;");

	jclass activity_class = env->FindClass("android/app/NativeActivity");
	jmethodID activity_get_package_resource_path = env->GetMethodID(activity_class, "getPackageResourcePath", "()Ljava/lang/String;");
	jmethodID activity_get_files_dir = env->GetMethodID(activity_class, "getFilesDir", "()Ljava/io/File;");
	jmethodID activity_get_cache_dir = env->GetMethodID(activity_class, "getCacheDir", "()Ljava/io/File;");

	int count = lua_gettop(L);
	const char *file_path = NULL;
	if (!lua_isnil(L, 1)) {
		file_path = lua_tostring(L, 1);
	}

	jstring directory_path_string = NULL;
	SystemDirectory directory = system_directory_resource;
	if (count == 2) {
		directory = (SystemDirectory)lua_tonumber(L, 2);
	}
	switch (directory) {
		case system_directory_resource:
			directory_path_string = (jstring)env->CallObjectMethod(dmGraphics::GetNativeAndroidActivity(), activity_get_package_resource_path);
			break;
		case system_directory_caches:
		case system_directory_temporary:
			{
				jobject cache_file_object = env->CallObjectMethod(dmGraphics::GetNativeAndroidActivity(), activity_get_cache_dir);
				directory_path_string = (jstring)env->CallObjectMethod(cache_file_object, file_get_canonical_path);
				if (directory == system_directory_temporary) {
					jstring tmp_string = env->NewStringUTF("/tmp");
					directory_path_string = (jstring)env->CallObjectMethod(directory_path_string, string_concat, tmp_string);
					env->DeleteLocalRef(tmp_string);
					jobject tmp_file_object = env->NewObject(file_class, file_constructor, directory_path_string);
					if (!env->CallBooleanMethod(tmp_file_object, file_exists)) {
						env->CallBooleanMethod(tmp_file_object, file_mkdirs);
					}
				}
			}
			break;
		case system_directory_documents:
			{
				jobject files_file_object = env->CallObjectMethod(dmGraphics::GetNativeAndroidActivity(), activity_get_files_dir);
				directory_path_string = (jstring)env->CallObjectMethod(files_file_object, file_get_canonical_path);
			}
			break;
		default:
			dmLogError("Unsupported directory.");
			return 0;
	}
	const char *directory_path = env->GetStringUTFChars(directory_path_string, 0);
	if (file_path) {
		char full_path[strlen(directory_path) + strlen(file_path) + 1];
		sprintf(full_path, "%s/%s", directory_path, file_path);
		lua_pushstring(L, full_path);
	} else {
		lua_pushstring(L, directory_path);
	}
	env->ReleaseStringUTFChars(directory_path_string, directory_path);
	return 1;
}

dmExtension::Result APP_INITIALIZE(dmExtension::AppParams *params) {
	dmLogInfo("APP_INITIALIZE");
	return dmExtension::RESULT_OK;
}

dmExtension::Result APP_FINALIZE(dmExtension::AppParams *params) {
	dmLogInfo("APP_FINALIZE");
	// TODO: delete tmp dir.
	return dmExtension::RESULT_OK;
}

dmExtension::Result INITIALIZE(dmExtension::Params *params) {
	dmLogInfo("INITIALIZE");
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
	dmLogInfo("FINALIZE");
	return dmExtension::RESULT_OK;
}

DECLARE_DEFOLD_EXTENSION

#endif
