package = 'LuaHV'
version = '1.0-1'
source = {
	url = 'git://github.com/iskolbin/luahv',
	tag = 'v1.0',
}
description = {
	summary = 'Homogeneous vectors for Lua',
	detailed = [[]],
	homepage = 'https://github.com/iskolbin/luahv',
	license = 'MIT/X11',
}
dependencies = {
	'lua >= 5.1'
}
build = {
	type = 'builtin',
	modules = {
		hv = {
			sources = {
				'hv.c',
			},
		}
	}
}
