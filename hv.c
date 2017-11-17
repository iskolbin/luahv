#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* start indexing at 1: HV_ORIGIN 1 */
#define HV_ORIGIN 0

#if (LUA_VERSION_NUM<=501)
#define lua_len(L,i) (lua_pushnumber( (L), lua_objlen( (L), (i) )))
#define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))

static void *luaL_testudata (lua_State *L, int ud, const char *tname);
static void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);

void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  for (; l->name != NULL; l++) {  /* fill the table with given functions */
    int i;
    for (i = 0; i < nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -nup);
    lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_pop(L, nup);  /* remove upvalues */
}

void *luaL_testudata (lua_State *L, int ud, const char *tname) {
	void *p = lua_touserdata(L, ud);
	if (p != NULL) {  /* value is a userdata? */
		if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
			luaL_getmetatable(L, tname);  /* get correct metatable */
			if (!lua_rawequal(L, -1, -2))  /* not the same? */
				p = NULL;  /* value is a userdata with wrong metatable */
			lua_pop(L, 2);  /* remove both metatables */
			return p;
		}
	}
	return NULL;  /* value is not a userdata with a metatable */
}
#endif

#ifdef _WIN32
__declspec (dllexport)
#endif
int luaopen_luahv( lua_State *L );

#define HV_LOAD_VECTOR_MT(TYPE) \
	luaL_newmetatable( L, #TYPE ".vector" ); \
	luaL_setfuncs( L, vectormt_##TYPE, 0 ); \

#define HV_ADD_VECTOR_MT(NAME,TYPE) {NAME, newvector##TYPE}

#define HV_BINARY_OP(TYPE) \
	vector##TYPE *vec1 = luaL_checkudata( L, 1, #TYPE ".vector" ); \
	vector##TYPE *vec2 = luaL_checkudata( L, 2, #TYPE ".vector" ); \
	vector##TYPE *res; \
	size_t i;\
	luaL_argcheck( L, vec1 != NULL, 1, "'" #TYPE ".vector' expected" ); \
	luaL_argcheck( L, vec2 != NULL, 2, "'" #TYPE ".vector' expected" ); \

#define HV_BINARY_CHECK_SIZE \
	if ( vec1->size != vec2->size ) { \
		luaL_error( L, "vectors must have the same size" ); \
		return 0; \
	} \

#define HV_BINARY_EVAL(TYPE,OPERATION) { \
	HV_BINARY_OP(TYPE) \
	HV_BINARY_CHECK_SIZE \
	res = makevector##TYPE( L, vec1->size ); \
	for ( i = 0; i < res->size; i++ ) { \
		res->values[i] = vec1->values[i] OPERATION vec2->values[i]; \
	} \
	return 1; }

#define HV_UNARY_EVAL(TYPE,OPERATION) { \
	vector##TYPE *vec1 = luaL_checkudata( L, 1, #TYPE ".vector" ); \
	vector##TYPE *res; \
	size_t i;\
	luaL_argcheck( L, vec1 != NULL, 1, "'" #TYPE ".vector' expected" ); \
	res = makevector##TYPE( L, vec1->size ); \
	for ( i = 0; i < res->size; i++ ) { \
		res->values[i] = OPERATION vec1->values[i]; \
	} \
	return 1; }
		
#define HV_MAKE_VECTOR(TYPE,TYPE_POP,TYPE_PUSH) \
\
typedef struct {  \
	size_t size; \
	TYPE values[1]; \
} vector##TYPE ; \
\
static vector##TYPE *makevector##TYPE( lua_State *L, int n ) { \
	size_t i; \
	vector##TYPE *vec = lua_newuserdata( L, sizeof( *vec ) + n*sizeof( TYPE )); \
	vec->size = n; \
	luaL_getmetatable( L, #TYPE ".vector" ); \
	lua_setmetatable( L, -2 ); \
	return vec; \
} \
\
static int newvector##TYPE ( lua_State *L ) { \
	size_t i; \
	vector##TYPE *vec; \
	size_t n = luaL_checkint( L, 1 ); \
	TYPE v = lua_tonumber( L, 2 ); \
	luaL_argcheck( L, n >= 1, 1, "invalid size" ); \
	vec = makevector##TYPE( L, n ); \
	for ( i = 0; i < n; i++ ) vec->values[i] = v; \
	return 1; \
} \
\
static int setvector##TYPE ( lua_State *L ) { \
	vector##TYPE *vec = luaL_checkudata( L, 1, #TYPE ".vector" ); \
	size_t index = luaL_checkint( L, 2 ) - HV_ORIGIN; \
	luaL_argcheck( L, vec != NULL, 1, "'" #TYPE ".vector' expected" ); \
	luaL_argcheck( L, 0 <= index && index < vec->size, 2, "index out of range" ); \
	vec->values[index] = TYPE_POP( L, 3 ); \
	return 0; \
} \
\
static int getvector##TYPE ( lua_State *L ) { \
	vector##TYPE *vec = luaL_checkudata( L, 1, #TYPE ".vector" ); \
	size_t index = luaL_checkint( L, 2 ) - HV_ORIGIN; \
	luaL_argcheck( L, vec != NULL, 1, "'" #TYPE ".vector' expected" ); \
	luaL_argcheck( L, 0 <= index && index < vec->size, 2, "index out of range" ); \
	TYPE_PUSH( L, vec->values[index] ); \
	return 1;	\
} \
\
static int sizevector##TYPE ( lua_State *L ) { \
	vector##TYPE *vec = luaL_checkudata( L, 1, #TYPE ".vector" ); \
	luaL_argcheck( L, vec != NULL, 1, "'" #TYPE ".vector' expected" ); \
	lua_pushinteger( L, vec->size ); \
	return 1;	\
} \
\
static int addvector##TYPE ( lua_State *L ) HV_BINARY_EVAL(TYPE,+) \
static int subvector##TYPE ( lua_State *L ) HV_BINARY_EVAL(TYPE,-) \
static int mulvector##TYPE ( lua_State *L ) HV_BINARY_EVAL(TYPE,*) \
static int divvector##TYPE ( lua_State *L ) HV_BINARY_EVAL(TYPE,/) \
\
static int concatvector##TYPE ( lua_State *L ) { \
	HV_BINARY_OP(TYPE) \
	res = makevector##TYPE( L, vec1->size + vec2->size ); \
	for ( i = 0; i < vec1->size; i++ ) { \
		res->values[i] = vec1->values[i]; \
	} \
	for ( i = 0; i < vec2->size; i++ ) { \
		res->values[i+vec1->size] = vec2->values[i]; \
	} \
	return 1; \
} \
\
static int eqvector##TYPE ( lua_State *L ) { \
	HV_BINARY_OP(TYPE) \
	HV_BINARY_CHECK_SIZE \
	for ( i = 0; i < vec1->size; i++ ) {\
		if ( vec1->values[i] != vec2->values[i] ) { \
			lua_pushboolean( L, 0 ); \
			return 1; \
		}\
	}\
	lua_pushboolean( L, 1 ); \
	return 1; \
} \
\
static int unmvector##TYPE ( lua_State *L ) {\
	vector##TYPE *vec = luaL_checkudata( L, 1, #TYPE ".vector" ); \
	vector##TYPE *res; \
	size_t i;\
	luaL_argcheck( L, vec != NULL, 1, "'" #TYPE ".vector' expected" ); \
	res = makevector##TYPE( L, vec->size ); \
	for ( i = 0; i < res->size; i++ ) { \
		res->values[i] = -vec->values[i]; \
	} \
	return 1; \
}\
\
static int tostringvector##TYPE ( lua_State *L ) { \
	vector##TYPE *vec = luaL_checkudata( L, 1, #TYPE ".vector" ); \
	size_t i;\
	luaL_argcheck( L, vec != NULL, 1, "'" #TYPE ".vector' expected" ); \
	lua_pushstring( L, "{" ); \
	for ( i = 0; i < vec->size; i++ ) { \
		lua_pushnumber( L, vec->values[i] ); \
		lua_pushstring( L, "," ); \
	} \
	lua_pushstring( L, "}/" #TYPE ); \
	lua_concat( L, vec->size*2 + 2 ); \
	return 1; \
} \
\
static int totable##TYPE( lua_State *L ) { \
	vector##TYPE *vec = luaL_checkudata( L, 1, #TYPE ".vector" ); \
	size_t i;\
	luaL_argcheck( L, vec != NULL, 1, "'" #TYPE ".vector' expected" ); \
	lua_newtable(L); \
	for ( i = 0; i < vec->size; i++ ) { \
		lua_pushnumber( L, vec->values[i] ); \
		lua_rawseti( L, -2, i+1 ); \
	} \
	return 1; \
} \
\
static int from##TYPE( lua_State *L ) { \
	if ( lua_istable( L, 1 )) {\
		size_t i, n;\
	 	vector##TYPE *res;\
		lua_len( L, 1 );\
		n = lua_tointeger( L, -1 );\
		res = makevector##TYPE( L, n );\
		for ( i = 1; i <= n; i++ ) {\
			lua_rawgeti( L, 1, i );\
			res->values[i-1] = (TYPE) lua_tonumber( L, -1 );\
		}\
		lua_pop(L,n);\
		return 1;\
	} else { \
		return 0;\
	}\
}\
\
static int is##TYPE ( lua_State *L ) { \
	if ( luaL_testudata( L, 1, #TYPE ".vector" )) { \
		lua_pushboolean( L, 1 ); \
	} else { \
		lua_pushboolean( L, 0 ); \
	} \
	return 1; \
}

#define HV_MAKE_INT_VECTOR(TYPE,POP,PUSH) HV_MAKE_VECTOR(TYPE,POP,PUSH) \
static int mod##TYPE ( lua_State *L ) HV_BINARY_EVAL(TYPE,%) \
static int not##TYPE ( lua_State *L ) HV_UNARY_EVAL(TYPE,~)  \
static int and##TYPE ( lua_State *L ) HV_BINARY_EVAL(TYPE,&) \
static int xor##TYPE ( lua_State *L ) HV_BINARY_EVAL(TYPE,^) \
static int or##TYPE  ( lua_State *L ) HV_BINARY_EVAL(TYPE,|) \
static int rshift##TYPE ( lua_State *L ) HV_BINARY_EVAL(TYPE,>>) \
static int lshift##TYPE ( lua_State *L ) HV_BINARY_EVAL(TYPE,<<) \
static const struct luaL_Reg vectormt_##TYPE [] = { \
	{"__newindex", setvector##TYPE}, \
	{"__index", getvector##TYPE}, \
	{"__len", sizevector##TYPE}, \
	{"__add", addvector##TYPE}, \
	{"__sub", subvector##TYPE}, \
	{"__mul", mulvector##TYPE}, \
	{"__div", divvector##TYPE}, \
	{"__mod", mod##TYPE}, \
	{"__concat", concatvector##TYPE}, \
	{"__eq", eqvector##TYPE}, \
	{"__tostring", tostringvector##TYPE}, \
	{"__unm", unmvector##TYPE}, \
	{NULL, NULL} \
};

#define HV_MAKE_FLOAT_VECTOR(TYPE,POP,PUSH) HV_MAKE_VECTOR(TYPE,POP,PUSH) \
static const struct luaL_Reg vectormt_##TYPE [] = { \
	{"__newindex", setvector##TYPE}, \
	{"__index", getvector##TYPE}, \
	{"__len", sizevector##TYPE}, \
	{"__add", addvector##TYPE}, \
	{"__sub", subvector##TYPE}, \
	{"__mul", mulvector##TYPE}, \
	{"__div", divvector##TYPE}, \
	{"__concat", concatvector##TYPE}, \
	{"__eq", eqvector##TYPE}, \
	{"__tostring", tostringvector##TYPE}, \
	{"__unm", unmvector##TYPE}, \
	{NULL, NULL} \
};

HV_MAKE_INT_VECTOR( int8_t, luaL_checknumber, lua_pushnumber );
HV_MAKE_INT_VECTOR( int16_t, luaL_checknumber, lua_pushnumber );
HV_MAKE_INT_VECTOR( int32_t, luaL_checknumber, lua_pushnumber );
HV_MAKE_INT_VECTOR( int64_t, luaL_checknumber, lua_pushnumber );
HV_MAKE_INT_VECTOR( uint8_t, luaL_checknumber, lua_pushnumber );
HV_MAKE_INT_VECTOR( uint16_t, luaL_checknumber, lua_pushnumber );
HV_MAKE_INT_VECTOR( uint32_t, luaL_checknumber, lua_pushnumber );
HV_MAKE_INT_VECTOR( uint64_t, luaL_checknumber, lua_pushnumber );
HV_MAKE_FLOAT_VECTOR( float, luaL_checknumber, lua_pushnumber );
HV_MAKE_FLOAT_VECTOR( double, luaL_checknumber, lua_pushnumber );

#define HV_MAKE_EXPAND_INT(ARG) \
	{  "i8" #ARG, ARG##int8_t   },\
	{ "i16" #ARG, ARG##int16_t  },\
	{ "i32" #ARG, ARG##int32_t  },\
	{ "i64" #ARG, ARG##int64_t  },\
	{  "u8" #ARG, ARG##uint8_t  },\
	{ "u16" #ARG, ARG##uint16_t },\
	{ "u32" #ARG, ARG##uint32_t },\
	{ "u64" #ARG, ARG##uint64_t },

#define HV_MAKE_EXPAND_FLOAT(ARG) \
	{ #ARG "f32", ARG##float  },\
	{ #ARG "f64", ARG##double },

static const struct luaL_Reg hvlib [] = {
	HV_ADD_VECTOR_MT( "i8", int8_t ),
	HV_ADD_VECTOR_MT( "i16", int16_t ),
	HV_ADD_VECTOR_MT( "i32", int32_t ),
	HV_ADD_VECTOR_MT( "i64", int64_t ),
	HV_ADD_VECTOR_MT( "u8", uint8_t ),
	HV_ADD_VECTOR_MT( "u16", uint16_t ),
	HV_ADD_VECTOR_MT( "u32", uint32_t ),
	HV_ADD_VECTOR_MT( "u64", uint64_t ),
	HV_ADD_VECTOR_MT( "f32", float ),
	HV_ADD_VECTOR_MT( "f64", double ),
	HV_MAKE_EXPAND_INT(is)
	HV_MAKE_EXPAND_INT(totable)
	HV_MAKE_EXPAND_INT(lshift)
	HV_MAKE_EXPAND_INT(rshift)
	HV_MAKE_EXPAND_INT(not)
	HV_MAKE_EXPAND_INT(xor)
	HV_MAKE_EXPAND_INT(or)
	HV_MAKE_EXPAND_FLOAT(is)
	HV_MAKE_EXPAND_FLOAT(totable)
	{NULL, NULL}
};

int luaopen_hv( lua_State *L ) {
	HV_LOAD_VECTOR_MT( int8_t );
	HV_LOAD_VECTOR_MT( int16_t );
	HV_LOAD_VECTOR_MT( int32_t );
	HV_LOAD_VECTOR_MT( int64_t );
	HV_LOAD_VECTOR_MT( uint8_t );
	HV_LOAD_VECTOR_MT( uint16_t );
	HV_LOAD_VECTOR_MT( uint32_t );
	HV_LOAD_VECTOR_MT( uint64_t );
	HV_LOAD_VECTOR_MT( float );
	HV_LOAD_VECTOR_MT( double );
	luaL_newlib( L, hvlib );
	return 1;
}
