#pragma once
#include <gsl.hpp>
#include <string>

std::string get_home_directory();
std::string get_current_directory();
bool is_path_absolute(gsl::cstring_span path);
bool is_path_separator(char32_t character);
void append_path_separator(std::string &path);
std::string normalize_path_separators(gsl::cstring_span path);
std::string make_path_absolute(gsl::cstring_span path);
gsl::cstring_span path_file_name(gsl::cstring_span path);
gsl::cstring_span path_directory(gsl::cstring_span path);

std::string get_display_path(gsl::cstring_span path);
