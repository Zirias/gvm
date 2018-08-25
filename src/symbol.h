#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdint.h>

typedef struct Symbol Symbol;
typedef struct SymTable SymTable;
typedef struct SymIter SymIter;

SymTable *SymTable_create(void);
Symbol *SymTable_symbol(SymTable *self, const char *name);
const char *Symbol_name(const Symbol *self);
int Symbol_resolved(const Symbol *self);
uint16_t Symbol_value(const Symbol *self);
void Symbol_setValue(Symbol *self, uint16_t val);
SymIter *SymTable_createIter(const SymTable *self);
int SymIter_moveNext(SymIter *self);
const Symbol *SymIter_current(const SymIter *self);
void SymIter_destroy(SymIter *self);
void SymTable_destroy(SymTable *self);

#endif
