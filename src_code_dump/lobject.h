/*
** $Id: lobject.h,v 2.20.1.2 2008/08/06 13:29:48 roberto Exp $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#ifndef lobject_h
#define lobject_h


#include <stdarg.h>


#include "llimits.h"
#include "lua.h"


/* tags for values visible from Lua */
#define LAST_TAG	LUA_TTHREAD//协程 8

#define NUM_TAGS	(LAST_TAG+1) //9


/*
** Extra tags for non-values
*/
#define LUA_TPROTO	(LAST_TAG+1)//9原型
#define LUA_TUPVAL	(LAST_TAG+2)//10高价值
#define LUA_TDEADKEY	(LAST_TAG+3)//11废键


/*
** Union of all collectable objects
*/
typedef union GCObject GCObject;//lstate.h  143行


/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#define CommonHeader	GCObject *next; lu_byte tt; lu_byte marked


/*
** Common header in struct form
*/
typedef struct GCheader {
  CommonHeader;
} GCheader;




/*
** Union of all Lua values
*/
typedef union {
  GCObject *gc;//GC类型大于等于4
  void *p;//轻量级指针2
  lua_Number n;//浮点3
  int b;//布尔1
} Value;//所有数据类型


/*
** Tagged Values
*/

#define TValuefields	Value value; int tt//数据内容及其类型

typedef struct lua_TValue {
  TValuefields;
} TValue;


/* Macros to test type *///识别类型
#define ttisnil(o)	(ttype(o) == LUA_TNIL)//是空类型吗0
#define ttisnumber(o)	(ttype(o) == LUA_TNUMBER)//是浮点吗3
#define ttisstring(o)	(ttype(o) == LUA_TSTRING)//是字符串吗4
#define ttistable(o)	(ttype(o) == LUA_TTABLE)//是表吗5
#define ttisfunction(o)	(ttype(o) == LUA_TFUNCTION)//是函数吗6
#define ttisboolean(o)	(ttype(o) == LUA_TBOOLEAN)//是布尔吗1
#define ttisuserdata(o)	(ttype(o) == LUA_TUSERDATA)//是指针吗7
#define ttisthread(o)	(ttype(o) == LUA_TTHREAD)//是协程吗8
#define ttislightuserdata(o)	(ttype(o) == LUA_TLIGHTUSERDATA)//是轻量级指针吗2

/* Macros to access values *///提取类型值
#define ttype(o)	((o)->tt)//存储的类型
#define gcvalue(o)	check_exp(iscollectable(o), (o)->value.gc)//GC值60行
#define pvalue(o)	check_exp(ttislightuserdata(o), (o)->value.p)//轻量级指针61行
#define nvalue(o)	check_exp(ttisnumber(o), (o)->value.n)//浮点值62行
#define rawtsvalue(o)	check_exp(ttisstring(o), &(o)->value.gc->ts)//字符串lstate.h 145行
#define tsvalue(o)	(&rawtsvalue(o)->tsv)//字符串206行
#define rawuvalue(o)	check_exp(ttisuserdata(o), &(o)->value.gc->u)//指针lstate.h 146行
#define uvalue(o)	(&rawuvalue(o)->uv)//指针222行
#define clvalue(o)	check_exp(ttisfunction(o), &(o)->value.gc->cl)//函数lstate.h 147行
#define hvalue(o)	check_exp(ttistable(o), &(o)->value.gc->h)//函数表lstate.h 148行
#define bvalue(o)	check_exp(ttisboolean(o), (o)->value.b)//布尔 63行
#define thvalue(o)	check_exp(ttisthread(o), &(o)->value.gc->th)//协程lstate.h 151行

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0))

/*
** for internal debug only
*/
#define checkconsistency(obj) \
  lua_assert(!iscollectable(obj) || (ttype(obj) == (obj)->value.gc->gch.tt))

#define checkliveness(g,obj) \
  lua_assert(!iscollectable(obj) || \
  ((ttype(obj) == (obj)->value.gc->gch.tt) && !isdead(g, (obj)->value.gc)))


/* Macros to set values *///初始化类型
#define setnilvalue(obj) ((obj)->tt=LUA_TNIL)//空类型0

#define setnvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.n=(x); i_o->tt=LUA_TNUMBER; }//浮点3

#define setpvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.p=(x); i_o->tt=LUA_TLIGHTUSERDATA; }//轻量级指针2

#define setbvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.b=(x); i_o->tt=LUA_TBOOLEAN; }//布尔1

#define setsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TSTRING; \
    checkliveness(G(L),i_o); }//字符串4

#define setuvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TUSERDATA; \
    checkliveness(G(L),i_o); }//指针7

#define setthvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTHREAD; \
    checkliveness(G(L),i_o); }//协程8

#define setclvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TFUNCTION; \
    checkliveness(G(L),i_o); }//函数6

#define sethvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTABLE; \
    checkliveness(G(L),i_o); }//表5

#define setptvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TPROTO; \
    checkliveness(G(L),i_o); }//原型9




#define setobj(L,obj1,obj2) \
  { const TValue *o2=(obj2); TValue *o1=(obj1); \
    o1->value = o2->value; o1->tt=o2->tt; \
    checkliveness(G(L),o1); }//初始化为obj2的类型


/*
** different types of sets, according to destination
*/

/* from stack to (same) stack */
#define setobjs2s	setobj
/* to stack (not from same stack) */
#define setobj2s	setobj
#define setsvalue2s	setsvalue
#define sethvalue2s	sethvalue
#define setptvalue2s	setptvalue
/* from table to same table */
#define setobjt2t	setobj
/* to table */
#define setobj2t	setobj
/* to new object */
#define setobj2n	setobj
#define setsvalue2n	setsvalue

#define setttype(obj, tt) (ttype(obj) = (tt))

// 只有这些类型的数据 才是可回收的数据
#define iscollectable(o)	(ttype(o) >= LUA_TSTRING)//是GC数据吗？大于等于4



typedef TValue *StkId;  /* index to stack elements *///栈元素的索引


/*
** String headers for string table
*/
typedef union TString {
  L_Umaxalign dummy;  /* ensures maximum alignment for strings *///替身
  struct {
    CommonHeader;
    lu_byte reserved;
    unsigned int hash;
    size_t len;
  } tsv;//Tstring values
} TString;//字符串类型4


#define getstr(ts)	cast(const char *, (ts) + 1)
#define svalue(o)       getstr(rawtsvalue(o))


// 这里为什么需要使用union类型？
typedef union Udata {
  L_Umaxalign dummy;  /* ensures maximum alignment for `local' udata *///替身
  struct {
    CommonHeader;
    struct Table *metatable;
    struct Table *env;
    size_t len;
  } uv;
} Udata;//指针7




/*
** Function Prototypes
*/
// 存放函数原型的数据结构
typedef struct Proto {
  CommonHeader;
  TValue *k;  /* constants used by the function */
  // 存放函数体的opcode
  Instruction *code;
  // 在这个函数中定义的函数
  struct Proto **p;  /* functions defined inside the function */
  int *lineinfo;  /* map from opcodes to source lines */
  // 存放局部变量的数组
  struct LocVar *locvars;  /* information about local variables */
  TString **upvalues;  /* upvalue names */
  TString  *source;
  int sizeupvalues;
  int sizek;  /* size of `k' */
  int sizecode;
  int sizelineinfo;
  int sizep;  /* size of `p' */
  int sizelocvars;
  int linedefined;
  int lastlinedefined;
  GCObject *gclist;
  lu_byte nups;  /* number of upvalues */
  lu_byte numparams;
  lu_byte is_vararg;
  lu_byte maxstacksize;
} Proto;//原型9


/* masks for new-style vararg */
#define VARARG_HASARG		1
#define VARARG_ISVARARG		2
#define VARARG_NEEDSARG		4

// 存放局部变量的结构体
typedef struct LocVar {
  TString *varname;
  int startpc;  /* first point where variable is active */
  int endpc;    /* first point where variable is dead */
} LocVar;



/*
** Upvalues
*/

typedef struct UpVal {
  CommonHeader;
  TValue *v;  /* points to stack or to its own value */
  union {
	// 当这个upval被close时,保存upval的值,后面可能还会被引用到
    TValue value;  /* the value (when closed) */
    // 当这个upval还在open状态时,以下链表串连在openupval链表中
    struct {  /* double linked list (when open) */
      struct UpVal *prev;
      struct UpVal *next;
    } l;
  } u;
} UpVal;//高价值


/*
** Closures闭包
*/

#define ClosureHeader \
	CommonHeader; lu_byte isC; lu_byte nupvalues; GCObject *gclist; \
	struct Table *env

typedef struct CClosure {
  ClosureHeader;
  lua_CFunction f;
  TValue upvalue[1];
} CClosure;//闭包C函数


typedef struct LClosure {
  ClosureHeader;
  struct Proto *p;
  UpVal *upvals[1];
} LClosure;//闭包lua函数


typedef union Closure {
  CClosure c;
  LClosure l;
} Closure;//闭包统称6


#define iscfunction(o)	(ttype(o) == LUA_TFUNCTION && clvalue(o)->c.isC)//是不是C函数？
#define isLfunction(o)	(ttype(o) == LUA_TFUNCTION && !clvalue(o)->c.isC)//是不是lua函数？


/*
** Tables
*/

typedef union TKey {
  struct {
    TValuefields;//Value value; int tt
    struct Node *next;  /* for chaining */
  } nk;
  TValue tvk;
} TKey;//表键

// 每个节点都有key和val
typedef struct Node {
  TValue i_val;//值
  TKey i_key;//键
} Node;//表结点


typedef struct Table {
  CommonHeader;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */ 
  lu_byte lsizenode;  /* log2 of size of `node' array */
  struct Table *metatable;
  TValue *array;  /* array part */
  Node *node;
  Node *lastfree;  /* any free position is before this position */
  GCObject *gclist;
  int sizearray;  /* size of `array' array */
} Table;//表5



/*
** `module' operation for hashing (size is always a power of 2)
*/
// (size&(size-1))==0是检查size是2的次幂
// (s) & ((size)-1)) = s % size
#define lmod(s,size) \
	(check_exp((size&(size-1))==0, (cast(int, (s) & ((size)-1)))))


#define twoto(x)	(1<<(x))
// sizenode返回的值必然是2的次幂
#define sizenode(t)	(twoto((t)->lsizenode))


#define luaO_nilobject		(&luaO_nilobject_)

LUAI_DATA const TValue luaO_nilobject_;

#define ceillog2(x)	(luaO_log2((x)-1) + 1)

LUAI_FUNC int luaO_log2 (unsigned int x);
LUAI_FUNC int luaO_int2fb (unsigned int x);
LUAI_FUNC int luaO_fb2int (int x);
LUAI_FUNC int luaO_rawequalObj (const TValue *t1, const TValue *t2);
LUAI_FUNC int luaO_str2d (const char *s, lua_Number *result);
LUAI_FUNC const char *luaO_pushvfstring (lua_State *L, const char *fmt,
                                                       va_list argp);
LUAI_FUNC const char *luaO_pushfstring (lua_State *L, const char *fmt, ...);
LUAI_FUNC void luaO_chunkid (char *out, const char *source, size_t len);


#endif

