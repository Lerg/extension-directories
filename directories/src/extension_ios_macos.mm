#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_OSX)

#include "extension.h"
#define DLIB_LOG_DOMAIN EXTENSION_NAME_STRING
#include <dmsdk/sdk.h>

#import <Foundation/Foundation.h>

#define EXTENSTION_INTERFACE FUNCTION_NAME_EXPANDED(EXTENSION_NAME, Interface)

@interface EXTENSTION_INTERFACE : NSObject
@property (retain) NSString *app_id;
@end

@implementation EXTENSTION_INTERFACE

typedef NS_ENUM(NSUInteger, SystemDirectory) {
	system_directory_application_support,
	system_directory_caches,
	system_directory_documents,
	system_directory_resource,
	system_directory_temporary
};

#if defined(DM_PLATFORM_IOS)
	static bool is_osx = false;
#else
	static bool is_osx = true;
#endif

static EXTENSTION_INTERFACE *extension;
static int path_for_file(lua_State *L) {return [extension path_for_file:L];}

-(int)open:(lua_State *)L {
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

	return 1;
}

-(int)path_for_file:(lua_State *)L {
	int count = lua_gettop(L);
	NSString *file_path = nil;
	if (!lua_isnil(L, 1)) {
		file_path = @(lua_tostring(L, 1));
	}
	NSString *directory_path = [[NSBundle mainBundle] resourcePath];
	if (count == 2) {
		SystemDirectory directory = (SystemDirectory)lua_tonumber(L, 2);
		if (directory != system_directory_resource) {
			switch (directory) {
				case system_directory_application_support:
					if (is_osx) {
						directory_path = [NSString stringWithFormat:@"%@/%@", [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0], extension.app_id];
					} else {
						directory_path = [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0];
					}
					break;
				case system_directory_caches:
					if (is_osx) {
						directory_path = [NSString stringWithFormat:@"%@/%@", [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0], extension.app_id];
					} else {
						directory_path = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0];
					}
					break;
				case system_directory_documents:
					if (is_osx) {
						directory_path = [NSString stringWithFormat:@"%@/%@/Documents", [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0], extension.app_id];
					} else {
						directory_path = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
					}
					break;
				case system_directory_temporary:
					if (is_osx) {
						directory_path = [NSString stringWithFormat:@"%@/%@", NSTemporaryDirectory(), extension.app_id];
					} else {
						directory_path = NSTemporaryDirectory();
					}
					break;
				default:
					dmLogError("Unsupported directory");
					return 0;
			}
			if (is_osx) {
				NSError *error;
				if (![[NSFileManager defaultManager] createDirectoryAtURL:[NSURL fileURLWithPath:directory_path] withIntermediateDirectories:YES attributes:nil error:&error]) {
					dmLogError("Failed to create directory %s.", [[error localizedDescription] UTF8String]);
				}
			}
		}
	}
	if (file_path) {
		lua_pushstring(L, [[directory_path stringByAppendingPathComponent:file_path] UTF8String]);
	} else {
		lua_pushstring(L, [directory_path UTF8String]);
	}
	return 1;
}

@end

dmExtension::Result APP_INITIALIZE(dmExtension::AppParams *params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result APP_FINALIZE(dmExtension::AppParams *params) {
	return dmExtension::RESULT_OK;
}

dmExtension::Result INITIALIZE(dmExtension::Params *params) {
	extension = [[EXTENSTION_INTERFACE alloc] init];
	[extension open:params->m_L];
	#if defined(DM_PLATFORM_OSX)
		const char *app_id = dmConfigFile::GetString(params->m_ConfigFile, "osx.bundle_identifier", NULL);
		if (app_id == NULL) {
			dmLogWarning("osx.bundle_identifier is not set, using project title instead.");
			app_id = dmConfigFile::GetString(params->m_ConfigFile, "project.title", "defold");
		}
		extension.app_id = @(app_id);
	#endif
	
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
