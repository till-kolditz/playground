#include <cassert>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <system_error>

#include <simdjson.h>

// NOLINTBEGIN(missing-includes)
using simdjson::padded_string;
using simdjson::simdjson_error;
using simdjson::simdjson_result;
using simdjson::ondemand::document;
using simdjson::ondemand::json_type;
using std::string_view;
using std::filesystem::filesystem_error;
using std::filesystem::path;
using jsonparser = simdjson::ondemand::parser;
// NOLINTEND(missing-includes)

using std::string_view_literals::operator""sv;

path sanitize_path(string_view sv);
simdjson_result<padded_string> load_json_file(path const &path);
simdjson_result<document>
load_as_document(jsonparser *parser,
                 simdjson_result<padded_string> const &json);
void print_document_structure(simdjson_result<document> *doc);

int main(int argc, char **args) {
  if (argc != 2) {
    std::cerr << "You must provide the path to a JSON file\n";
    return 1;
  }

  try {
    auto path = sanitize_path(args[1]);
    auto json = load_json_file(path);
    auto &parser = jsonparser::get_parser();
    auto doc = load_as_document(&parser, json);
    print_document_structure(&doc);
  } catch (simdjson_error const &exc) {
    std::cerr << exc.what() << '\n';
    return exc.error();
  } catch (filesystem_error const &exc) {
    std::cerr << exc.code() << ": "sv << exc.what() << '\n';
    return exc.code().value();
  }
  return 0;
}

path sanitize_path(string_view sv) {
  auto fspath = path{sv};
  if (!std::filesystem::exists(fspath)) {
    auto ec = std::make_error_code(std::errc::file_exists);
    auto exc = filesystem_error{"Path doesn't exist", fspath, ec};
    throw exc;
  }
  return fspath;
}

simdjson_result<padded_string> load_json_file(path const &fspath) {
  return padded_string::load(fspath.native());
}

simdjson_result<document>
load_as_document(jsonparser *parser,
                 simdjson_result<padded_string> const &json) {
  return parser->iterate(json);
}

void print_indentation(int);
template <typename T>
void print_generic(simdjson_result<T> *value, std::string_view key);
template <typename T> void print_error(simdjson_result<T> *value);
void print_indented_type(json_type type, int depth, std::string_view key);
template <typename T> void print_array(simdjson_result<T> *value);
template <typename T> void print_boolean(simdjson_result<T> *value);
template <typename T> void print_null(simdjson_result<T> *value);
template <typename T> void print_number(simdjson_result<T> *value);
template <typename T> void print_object(simdjson_result<T> *value);
template <typename T> void print_string(simdjson_result<T> *value);
template <typename T> void print_unknown(simdjson_result<T> *value);

void print_document_structure(simdjson_result<document> *doc) {
  std::cout << "JSON document structure:\n"sv;
  print_generic(doc, {});
}

template <typename T>
void print_generic(simdjson_result<T> *value, std::string_view key) {
  if (value->error()) {
    print_error(value);
  } else {
    print_indented_type(value->type(), value->current_depth(), key);

    switch (value->type()) {
    case json_type::array:
      print_array(value);
      break;
    case json_type::boolean:
      print_boolean(value);
      break;
    case json_type::null:
      print_null(value);
      break;
    case json_type::number:
      print_number(value);
      break;
    case json_type::object:
      print_object(value);
      break;
    case json_type::string:
      print_string(value);
      break;
    case json_type::unknown:
      print_unknown(value);
      break;
    }
  }
}

template <typename T> void print_error(simdjson_result<T> *value) {
  std::cout << "<error> "sv << value->error() << '\n';
}

void print_indented_type(json_type type, int depth, std::string_view key) {
  print_indentation(depth);
  std::cout << type;
  if (!key.empty()) {
    std::cout << " (" << key << ')';
  }
}

void print_indentation(int depth) {
  for (int i = 1; i < depth; ++i) {
    std::cout << '\t';
  }
}

template <typename T> void print_array(simdjson_result<T> *value) {
  assert(value->type() == json_type::array);
  int depth = value->current_depth();

  auto arr = value->get_array();
  std::cout << ' ' << arr->count_elements() << " elements [\n"sv;
  for (auto elem : arr) {
    print_generic(&elem, {});
  }
  print_indentation(depth);
  std::cout << "]\n";
}

template <typename T> void print_boolean(simdjson_result<T> *value) {
  assert(value->type() == json_type::boolean);
  std::cout << " : "
            << (static_cast<bool>(value->value()) ? "true"sv : "false"sv)
            << '\n';
}

template <typename T> void print_null(simdjson_result<T> *value) {
  assert(value->type() == json_type::null);
  std::cout << (value->value().is_null() ? "null"sv : "other"sv) << '\n';
}

template <typename T> void print_number(simdjson_result<T> *value) {
  assert(value->type() == json_type::number);
  std::cout << '\n';
}

template <typename T> void print_object(simdjson_result<T> *value) {
  assert(value->type() == json_type::object);
  if (value->has_value()) {
    int depth = value->current_depth();
    auto obj = value->get_object();
    std::cout << ' ' << obj.count_fields() << " fields {\n"sv;
    for (auto field : obj) {
      auto val = field.value();
      print_generic(&val, field.unescaped_key());
    }
    print_indentation(depth);
  } else {
    std::cout << " {"sv;
  }
  std::cout << "}\n"sv;
}

template <typename T> void print_string(simdjson_result<T> *value) {
  assert(value->type() == json_type::string);
  std::cout << '\n';
}

template <typename T> void print_unknown(simdjson_result<T> *value) {
  assert(value->type() == json_type::unknown);
  std::cout << '\n';
}
