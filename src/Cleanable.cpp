#include <assert.h>
#include <utility>
#include "Cleanable.h"

Cleanable::Cleanable() {
  cleanup_.function = nullptr;
  cleanup_.next = nullptr;
}

Cleanable::~Cleanable() { DoCleanup(); }

Cleanable::Cleanable(Cleanable&& other) {
  *this = std::move(other);
}

Cleanable& Cleanable::operator=(Cleanable&& other) {
  if (this != &other) {
    cleanup_ = other.cleanup_;
    other.cleanup_.function = nullptr;
    other.cleanup_.next = nullptr;
  }
  return *this;
}

void Cleanable::DelegateCleanupsTo(Cleanable* other) {
  assert(other != nullptr);
  if (cleanup_.function == nullptr) {
    return;
  }
  Cleanup* c = &cleanup_;
  other->RegisterCleanup(c->function, c->arg1, c->arg2);
  c = c->next;
  while (c != nullptr) {
    Cleanup* next = c->next;
    other->RegisterCleanup(c);
    c = next;
  }
  cleanup_.function = nullptr;
  cleanup_.next = nullptr;
}

void Cleanable::RegisterCleanup(Cleanable::Cleanup* c) {
  assert(c != nullptr);
  if (cleanup_.function == nullptr) {
    cleanup_.function = c->function;
    cleanup_.arg1 = c->arg1;
    cleanup_.arg2 = c->arg2;
    delete c;
  } else {
    c->next = cleanup_.next;
    cleanup_.next = c;
  }
}

void Cleanable::RegisterCleanup(CleanupFunction func, void* arg1, void* arg2) {
  assert(func != nullptr);
  Cleanup* c;
  if (cleanup_.function == nullptr) {
    c = &cleanup_;
  } else {
    c = new Cleanup;
    c->next = cleanup_.next;
    cleanup_.next = c;
  }
  c->function = func;
  c->arg1 = arg1;
  c->arg2 = arg2;
}

