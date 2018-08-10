/*
** $Id: lstring.c,v 2.8.1.1 2007/12/27 13:02:25 roberto Exp $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/


#include <string.h>

#define lstring_c
#define LUA_CORE

#include "lua.h"

#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"


// 对保存string的hash桶进行resize
void luaS_resize (lua_State *L, int newsize) {
  GCObject **newhash;//新散列表头首地址 lstate.h 39行
  stringtable *tb;//散列表头结构体 lstate.h 38行
  int i;
  if (G(L)->gcstate == GCSsweepstring)//如果处在回收阶段，直接返回
    return;  /* cannot resize during GC traverse */
  newhash = luaM_newvector(L, newsize, GCObject *);//新数组
  tb = &G(L)->strt;//旧数组结构
  for (i=0; i<newsize; i++) newhash[i] = NULL;//清空新数组
  /* rehash */
  for (i=0; i<tb->size; i++) {//遍历旧数组
    GCObject *p = tb->hash[i];//局部变量
    while (p) {  /* for each node in the list */
      GCObject *next = p->gch.next;  /* save next *///局部变量temp
      unsigned int h = gco2ts(p)->hash;//Tstring散列表hash值
      // 重新计算hash桶索引，这次需要mod新的hash桶大小
      int h1 = lmod(h, newsize);  /* new position *///找到新的位置index
      lua_assert(cast_int(h%newsize) == lmod(h, newsize));
      p->gch.next = newhash[h1];  /* chain it *///插入到新位置
      newhash[h1] = p;
      p = next;
    }
  }
  luaM_freearray(L, tb->hash, tb->size, TString *);
  tb->size = newsize;
  tb->hash = newhash;
}


static TString *newlstr (lua_State *L, const char *str, size_t l,
                                       unsigned int h) {
  TString *ts;//新节点
  stringtable *tb;//旧桶
  if (l+1 > (MAX_SIZET - sizeof(TString))/sizeof(char))
    luaM_toobig(L);//字符串是否超过最大长度
  ts = cast(TString *, luaM_malloc(L, (l+1)*sizeof(char)+sizeof(TString)));
  ts->tsv.len = l;//分配新空间，1个TString，后面是具体字符串值
  ts->tsv.hash = h;
  ts->tsv.marked = luaC_white(G(L));
  ts->tsv.tt = LUA_TSTRING;
  ts->tsv.reserved = 0;
  memcpy(ts+1, str, l*sizeof(char));//复制新字符串
  ((char *)(ts+1))[l] = '\0';  /* ending 0 *///初始化完毕
  tb = &G(L)->strt;//旧桶
  h = lmod(h, tb->size);//数组index
  ts->tsv.next = tb->hash[h];  /* chain new entry *///新节点
  tb->hash[h] = obj2gco(ts);//更新旧桶
  tb->nuse++;
  // 在hash桶数组大小小于MAX_INT/2的情况下，
  // 只要字符串数量大于桶数组数量就开始成倍的扩充桶的容量
  if (tb->nuse > cast(lu_int32, tb->size) && tb->size <= MAX_INT/2)
    luaS_resize(L, tb->size*2);  /* too crowded */
  return ts;
}//仅由lua_newlstr调用


TString *luaS_newlstr (lua_State *L, const char *str, size_t l) {
  GCObject *o;
  unsigned int h = cast(unsigned int, l);  /* seed *///强制类型转换l是字符串长度
  size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars */
  size_t l1;//除以32再加1得到步长
  for (l1=l; l1>=step; l1-=step)  /* compute hash *///随机hash值
    h = h ^ ((h<<5)+(h>>2)+cast(unsigned char, str[l1-1]));
  for (o = G(L)->strt.hash[lmod(h, G(L)->strt.size)];//桶数组index，o为GCobject类型
       o != NULL;
       o = o->gch.next) {
    TString *ts = rawgco2ts(o);//GCobject->ts
    if (ts->tsv.len == l && (memcmp(str, getstr(ts), l) == 0)) {//比较长度和字符串是否一样
      /* string may be dead */
      if (isdead(G(L), o)) changewhite(o);//判断是否准备收回
      return ts;
    }
  }
  return newlstr(L, str, l, h);  /* not found */
}//添加新字符串结点


Udata *luaS_newudata (lua_State *L, size_t s, Table *e) {
  Udata *u;
  if (s > MAX_SIZET - sizeof(Udata))
    luaM_toobig(L);
  u = cast(Udata *, luaM_malloc(L, s + sizeof(Udata)));
  u->uv.marked = luaC_white(G(L));  /* is not finalized */
  u->uv.tt = LUA_TUSERDATA;
  u->uv.len = s;
  u->uv.metatable = NULL;
  u->uv.env = e;
  /* chain it on udata list (after main thread) */
  // 这样让udata链接在mainthread之后，一定是整个GC链表的最后
  u->uv.next = G(L)->mainthread->next;
  G(L)->mainthread->next = obj2gco(u);
  return u;
}

