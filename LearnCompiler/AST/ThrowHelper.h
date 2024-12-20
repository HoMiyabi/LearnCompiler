#pragma once

#define THROW_IF_NULL(ptr) do {if ((ptr) == nullptr) throw std::runtime_error(#ptr ## " == nullptr");} while (0)