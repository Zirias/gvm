#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "symbol.h"

#define TABLESIZE 256
#define HASHMASK (TABLESIZE-1)

struct Symbol
{
    Symbol *next;
    char *name;
    int resolved;
    uint16_t val;
};

struct SymTable
{
    Symbol *buckets[TABLESIZE];
};

struct SymIter
{
    const SymTable *table;
    const Symbol *current;
    int currentBucket;
};

static unsigned hash(const char *str)
{
    unsigned h = 5381;
    unsigned char c;
    while ((c = *(const unsigned char *)str++))
    {
	h += (h << 5) + c;
    }
    return h & HASHMASK;
}

SymTable *SymTable_create(void)
{
    SymTable *self = calloc(1, sizeof *self);
    return self;
}

Symbol *SymTable_symbol(SymTable *self, const char *name)
{
    unsigned h = hash(name);
    Symbol *s = 0;
    Symbol *p = 0;
    for (s = self->buckets[h]; s; s=(p=s)->next)
    {
	if (!strcmp(name, s->name)) break;
    }
    if (s) return s;
    s = calloc(1, sizeof s);
    if (!s) return 0;
    s->name = malloc(strlen(name)+1);
    if (!s->name)
    {
	free(s);
	return 0;
    }
    strcpy(s->name, name);
    if (p) p->next = s;
    else self->buckets[h] = s;
    return s;
}

const char *Symbol_name(const Symbol *self)
{
    return self->name;
}

int Symbol_resolved(const Symbol *self)
{
    return self->resolved;
}

uint16_t Symbol_value(const Symbol *self)
{
    return self->val;
}

void Symbol_setValue(Symbol *self, uint16_t val)
{
    self->val = val;
    self->resolved = 1;
}

SymIter *SymTable_createIter(const SymTable *self)
{
    SymIter *i = malloc(sizeof *i);
    if (!i) return 0;
    i->table = self;
    i->current = 0;
    i->currentBucket = -1;
    return i;
}

int SymIter_moveNext(SymIter *self)
{
    if (self->current)
    {
	self->current = self->current->next;
    }
    while (!self->current)
    {
	if (++self->currentBucket == TABLESIZE)
	{
	    self->currentBucket = -1;
	    return 0;
	}
	self->current = self->table->buckets[self->currentBucket];
    }
    return 1;
}

const Symbol *SymIter_current(const SymIter *self)
{
    return self->current;
}

void SymIter_destroy(SymIter *self)
{
    free(self);
}

void SymTable_destroy(SymTable *self)
{
    if (!self) return;
    for (unsigned i = 0; i < TABLESIZE; ++i)
    {
	Symbol *n = 0;
	for (Symbol *s = self->buckets[i]; s; s=n)
	{
	    n = s->next;
	    free(s->name);
	    free(s);
	}
    }
    free(self);
}
