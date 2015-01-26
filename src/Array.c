#include "Cello.h"

static const char* Array_Name(void) {
  return "Array";
}

static const char* Array_Brief(void) {
  return "Sequential Container";
}

static const char* Array_Description(void) {
  return ""
    "Array is data structure containing a sequence of a single type of object. "
    "It can dynamically grow and shrink in size depending on how many elements "
    "it contains. It allocates storage for the type specified. It also "
    "deallocates and destroys the objects inside upon destruction."
    "\n\n"
    "Elements are copied in an Array using `assign` which means the type must "
    "implement the [Assign](documentation/assign) class."
    "\n\n"
    "Elements are ordered linearly. Elements are accessed by their position in "
    "this sequence directly. Addition and removal of elements at the end of "
    "the sequence is fast, with memory movement required for elements in the "
    "middle of the sequence." 
    "\n\n"
    "The equivalent C++ construct to this type is "
    "[std::vector](http://www.cplusplus.com/reference/vector/vector/)";
}

static const char* Array_Examples(void) {
  return ""
    "__Construction & Deletion__\n"
    "    \n"
    "    var x = new(Array, Int);\n"
    "    push(x, $(Int, 32));\n"
    "    push(x, $(Int, 6));\n"
    "    \n"
    "    /* <'Array' At 0x0000000000414603 [32, 6]> */\n"
    "    show(x);\n"
    "    del(x);\n"
    "    \n"
    "__Element Access__\n"
    "    \n"
    "    var x = new(Array, Real, $(Real, 0.01), $(Real, 5.12));\n"
    "    \n"
    "    show(get(x, $(Int, 0))); /* 0.01 */\n"
    "    show(get(x, $(Int, 1))); /* 5.12 */\n"
    "    \n"
    "    set(x, $(Int, 0), $(Real, 500.1));\n"
    "    show(get(x, $(Int, 0))); /* 500.1 */\n"
    "    \n"
    "    del(x);\n"
    "    \n"
    "__Collection Queries__\n"
    "    \n"
    "    var x = new(Array, Char,\n" 
    "        $(Char, 'a'), $(Char, 'b'), $(Char, 'c'), $(Char, 'd'));\n"
    "    \n"
    "    show(contains(x, $(Char, 'a'))); /* True */\n"
    "    show($(Int, len(x)));            /* 4 */\n"
    "    \n"
    "    discard(x, $(Char, 'c'));\n"
    "    \n"
    "    show(contains(x, $(Char, 'c'))); /* False */\n"
    "    show($(Int, len(x)));            /* 3 */\n"
    "    show(empty(x));               /* False */\n"
    "    \n"
    "    clear(x);\n"
    "    \n"
    "    show(empty(x));               /* True */\n"
    "    \n"
    "    del(x);\n"
    "    \n"
    "__Iteration__\n"
    "    \n"
    "    var greetings = new(Array, String, \n"
    "        $(String, \"Hello\"), \n"
    "        $(String, \"Bonjour\"), \n"
    "        $(String, \"Hej\")); \n"
    "    \n"
    "    foreach(greet in greetings) {\n"
    "      show(greet);\n"
    "    }\n"
    "    \n"
    "    del(x);\n";
}

/* TODO */
static const char* Array_Methods(void) {
  return "";
};

struct Array {
  var type;
  var data;
  size_t tsize;
  size_t nitems;
  size_t nslots;
};

static size_t Array_Step(struct Array* a) {
  return a->tsize + sizeof(struct CelloHeader);
}

static var Array_Item(struct Array* a, size_t i) {
  return a->data + Array_Step(a) * i + sizeof(struct CelloHeader);
}

static void Array_Alloc(struct Array* a, size_t i) {
  memset(a->data + Array_Step(a) * i, 0, Array_Step(a));
  struct CelloHeader* head = a->data + Array_Step(a) * i;
  head->type  = a->type;
  head->flags = (var)CelloDataAlloc;
}

static var Array_New(var self, var args) {
  
  struct Array* a = self;
  a->type   = cast(get(args, $(Int, 0)), Type);
  a->tsize  = size(a->type);
  a->nitems = len(args)-1;
  a->nslots = a->nitems;
  
  if (a->nslots is 0) {
    a->data = None;
    return self;
  }
  
  a->data = malloc(a->nslots * Array_Step(a));
  
  if (a->data is None) {
    throw(OutOfMemoryError, "Cannot allocate Array, out of memory!");
  }
  
  for(size_t i = 0; i < a->nitems; i++) {
    Array_Alloc(a, i);
    assign(Array_Item(a, i), get(args, $(Int, i+1)));  
  }
  
  return self;
}

static var Array_Del(var self) {
  
  struct Array* a = self;
  for(size_t i = 0; i < a->nitems; i++) {
    destruct(Array_Item(a, i));
  }
  
  free(a->data);
  return self;
  
}

static size_t Array_Size(void) {
  return sizeof(struct Array);
}

static void Array_Clear(var self) {
  struct Array* a = self;
  
  for(size_t i = 0; i < a->nitems; i++) {
    destruct(Array_Item(a, i));
  }
  
  free(a->data);
  a->data  = None;
  a->nitems = 0;
  a->nslots = 0;
}

static var Array_Assign(var self, var obj) {
  struct Array* a = self;

  Array_Clear(self);
  
  a->nitems = len(obj);
  a->nslots = a->nitems;
  
  if (a->nslots is 0) {
    a->data = None;
    return self;
  }
  
  a->data = malloc(a->nslots * Array_Step(a));
  
  if (a->data is None) {
    throw(OutOfMemoryError, "Cannot allocate Array, out of memory!");
  }
  
  for(size_t i = 0; i < a->nitems; i++) {
    Array_Alloc(a, i);
    assign(Array_Item(a, i), get(obj, $(Int, i)));  
  }
  
  return self;
  
}

static void Array_Reserve_More(struct Array* a) {
  
  if (a->nitems > a->nslots) {
    a->nslots = a->nitems + a->nitems / 2;
    a->data = realloc(a->data, Array_Step(a) * a->nslots);
    if (a->data is None) {
      throw(OutOfMemoryError, "Cannot grow Array, out of memory!");
    }
  }

}

static void Array_Concat(var self, var obj) {
  
  struct Array* a = self;
  
  size_t i = 0;
  size_t olen = len(obj);
  
  a->nitems += olen;
  Array_Reserve_More(a);
  
  foreach (item in obj) {
    Array_Alloc(a, a->nitems-olen+i);
    assign(Array_Item(a, a->nitems-olen+i), item);
    i++;
  }
  
}

static var Array_Copy(var self) { 
  struct Array* a = self;
  var b = new(Array, a->type);
  Array_Concat(b, self);
  return b;
}

static var Array_Eq(var self, var obj) {
  
  struct Array* a = self;
  if (a->nitems isnt len(obj)) { return False; }
  
  for(size_t i = 0; i < a->nitems; i++) {
    if_neq(Array_Item(a, i), get(obj, $(Int, i))) {
      return False;
    }
  }
  
  return True;
}

static size_t Array_Len(var self) {
  struct Array* a = self;
  return a->nitems;
}

static var Array_Mem(var self, var obj) {
  struct Array* a = self;
  for(size_t i = 0; i < a->nitems; i++) {
    if_eq(Array_Item(a, i), obj) {
      return True;
    }
  }
  return False;
}

static void Array_Reserve_Less(struct Array* a) {
  if (a->nslots > a->nitems + a->nitems / 2) {
    a->nslots = a->nitems;
    a->data = realloc(a->data, Array_Step(a) * a->nslots);
  }
}

static void Array_Pop_At(var self, var key) {

  struct Array* a = self;
  int64_t i = type_of(key) is Int ? ((struct Int*)key)->val : c_int(key);
  
#if CELLO_BOUNDS_CHECK == 1
  if (i < 0 or i >= a->nitems) {
    throw(IndexOutOfBoundsError,
      "Index '%i' out of bounds for Array of size %i.", key, $(Int, a->nitems));
    return;
  }
#endif
  
  destruct(Array_Item(a, i));
  
  memmove(a->data + Array_Step(a) * (i+0), 
          a->data + Array_Step(a) * (i+1), 
          Array_Step(a) * ((a->nitems-1) - i));
  
  a->nitems--;
  Array_Reserve_Less(a);
}

static void Array_Rem(var self, var obj) {
  struct Array* a = self;
  for(size_t i = 0; i < a->nitems; i++) {
    if_eq(Array_Item(a, i), obj) {
      Array_Pop_At(a, $(Int, i));
      return;
    }
  }
  throw(ValueError, "Object %$ not in Array!", obj);
}

static void Array_Push(var self, var obj) {
  struct Array* a = self;
  a->nitems++;
  Array_Reserve_More(a);
  Array_Alloc(a, a->nitems-1);
  assign(Array_Item(a, a->nitems-1), obj);
}

static void Array_Push_At(var self, var obj, var key) {
  struct Array* a = self;
  a->nitems++;
  Array_Reserve_More(a);
  
  int64_t i = type_of(key) is Int ? ((struct Int*)key)->val : c_int(key);
  memmove(a->data + Array_Step(a) * (i+1), 
          a->data + Array_Step(a) * (i+0), 
          Array_Step(a) * ((a->nitems-1) - i));
  
  Array_Alloc(self, i);
  assign(Array_Item(a, i), obj);
}

static void Array_Pop(var self) {

  struct Array* a = self;
  
#if CELLO_BOUNDS_CHECK == 1
  if (a->nitems is 0) {
    throw(IndexOutOfBoundsError, "Cannot pop. Array is empty!");
    return;
  }
#endif
  
  destruct(Array_Item(a, a->nitems-1));
  
  a->nitems--;
  Array_Reserve_Less(a);
}

static var Array_Get(var self, var key) {

  struct Array* a = self;
  int64_t i = type_of(key) is Int ? ((struct Int*)key)->val : c_int(key);
  
#if CELLO_BOUNDS_CHECK == 1
  if (i < 0 or i >= a->nitems) {
    return throw(IndexOutOfBoundsError,
      "Index '%i' out of bounds for Array of size %i.", key, $(Int, a->nitems));
  }
#endif
  
  return Array_Item(a, i);
}

static void Array_Set(var self, var key, var val) {

  struct Array* a = self;
  int64_t i = type_of(key) is Int ? ((struct Int*)key)->val : c_int(key);
  
#if CELLO_BOUNDS_CHECK == 1
  if (i < 0 or i >= a->nitems) {
    throw(IndexOutOfBoundsError, 
      "Index '%i' out of bounds for Array of size %i.", key, $(Int, a->nitems));
    return;
  }
#endif
  
  assign(Array_Item(a, i), val);
}

static var Array_Iter_Init(var self) {
  struct Array* a = self;
  if (a->nitems is 0) { return Terminal; }
  return Array_Item(a, 0);
}

static var Array_Iter_Next(var self, var curr) {
  struct Array* a = self;
  if (curr >= Array_Item(a, a->nitems-1)) {
    return Terminal;
  } else {
    return curr + Array_Step(a);
  }
}

static void Array_Swap(struct Array* a, size_t i, size_t j) {
  if (i == j) { return; }
  char swapspace[sizeof(struct CelloHeader) + a->tsize];
  memcpy(swapspace, a->data + i * Array_Step(a), Array_Step(a));
  memcpy(a->data + i * Array_Step(a), 
         a->data + j * Array_Step(a), Array_Step(a));
  memcpy(a->data + j * Array_Step(a), swapspace, Array_Step(a));
}

static void Array_Reverse(var self) {
  struct Array* a = self;
  
  size_t l = Array_Len(a);
  for(size_t i = 0; i < l / 2; i++) {
    Array_Swap(a, i, l-1-i);
  }
}

static size_t Array_Sort_Partition(struct Array* a, int64_t l, int64_t r) {
  
  int64_t p = l + (r - l) / 2;
  char swapspace[sizeof(struct CelloHeader) + a->tsize];
  memcpy(swapspace, a->data + p * Array_Step(a), Array_Step(a));
  
  Array_Swap(a, p, r);
  
  int64_t s = l;
  for (int64_t i = l; i < r; i++) {
    if_lt (Array_Get(a, $(Int, i)), swapspace + sizeof(struct CelloHeader)) {
      Array_Swap(a, i, s);
      s++;
    }
  }
  
  Array_Swap(a, s, r);
  
  return s;
}

static void Array_Sort_Part(struct Array* a, int64_t l, int64_t r) {
  if (l < r) {
    int64_t s = Array_Sort_Partition(a, l, r);
    Array_Sort_Part(a, l, s-1);
    Array_Sort_Part(a, s+1, r);
  }
}

static void Array_Sort(var self) {
  Array_Sort_Part(self, 0, Array_Len(self)-1);
}

static int Array_Show(var self, var output, int pos) {
  struct Array* a = self;
  pos = print_to(output, pos, "<'Array' At 0x%p [", self);
  for(int i = 0; i < a->nitems; i++) {
    pos = print_to(output, pos, "%$", Array_Item(a, i));
    if (i < a->nitems-1) { pos = print_to(output, pos, ", "); }
  }
  pos = print_to(output, pos, "]>");
  return pos;
}

var Array = typedecl(Array,
  typeclass(Doc,
    Array_Name,        Array_Brief,
    Array_Description, Array_Examples,
    Array_Methods),
  typeclass(New,      Array_New, Array_Del, Array_Size),
  typeclass(Assign,   Array_Assign),
  typeclass(Copy,     Array_Copy),
  typeclass(Eq,       Array_Eq),
  typeclass(Clear,    Array_Clear),
  typeclass(Push,
    Array_Push,       Array_Pop,
    Array_Push_At,    Array_Pop_At),
  typeclass(Len,      Array_Len),
  typeclass(Get,      Array_Get, Array_Set, Array_Mem, Array_Rem),
  typeclass(Iter,     Array_Iter_Init, Array_Iter_Next),
  typeclass(Reverse,  Array_Reverse),
  typeclass(Sort,     Array_Sort),
  typeclass(Show,     Array_Show, NULL));

  
