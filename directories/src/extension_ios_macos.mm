#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_OSX)

#include <dmsdk/sdk.h>
#include "extension.h"

#import <Foundation/Foundation.h>

#define EXTENSTION_INTERFACE FUNCTION_NAME_EXPANDED(EXTENSION_NAME, Interface)

@interface EXTENSTION_INTERFACE : NSObject
@end

@implementation EXTENSTION_INTERFACE

typedef NS_ENUM(NSUInteger, SystemDirectory) {
	system_directory_application_support,
	system_directory_caches,
	system_directory_documents,
	system_directory_resource,
	system_directory_temporary
};

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

// TODO: add package id on macOS and move documents inside app support.
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
					directory_path = [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0];
					break;
				case system_directory_caches:
					directory_path = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0];
					break;
				case system_directory_documents:
					directory_path = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
					break;
				case system_directory_temporary:
					directory_path = NSTemporaryDirectory();
					break;
				default:
					NSLog(@"Unsupported directory");
					return 0;
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
