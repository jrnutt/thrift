/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <map>
#include <fstream>
#include <string>
#include <vector>

#include "t_oop_generator.h"
#include "platform.h"
#include "version.h"

using std::map;
using std::ofstream;
using std::string;
using std::vector;

/**
 * Rust code generator.
 */
class t_rs_generator : public t_oop_generator {
 public:
  t_rs_generator(t_program* program,
                 const map<string, string>& parsed_options,
                 const string& option_string)
    : t_oop_generator(program)
  {
    (void) parsed_options;
    (void) option_string;
    out_dir_base_ = "gen-rs";
  }

  void init_generator();
  void close_generator();

  /**
   * Program-level generation functions
   */
  void generate_typedef(t_typedef*  ttypedef);
  void generate_enum(t_enum*     tenum);
  void generate_struct(t_struct*   tstruct);
  void generate_service(t_service*  tservice);

  string rs_autogen_comment();
  string rs_imports();

  string render_rs_type(t_type* type);

 private:
/*
  void generate_service_interface(t_service* tservice, string style);
  void generate_service_interface_factory(t_service* tservice, string style);
  void generate_service_null(t_service* tservice, string style);
  void generate_service_multiface(t_service* tservice);
*/
  void generate_service_helpers(t_service* tservice);
  void generate_service_client(t_service* tservice);
/*
  void generate_service_processor(t_service* tservice, string style);
  void generate_service_skeleton(t_service* tservice);
*/
  void generate_function_helpers(t_service* tservice, t_function* tfunction);

  void generate_struct_declaration(t_struct* tstruct,
                                   bool is_exception = false,
                                   bool pointers = false,
                                   bool read = true,
                                   bool write = true,
                                   bool swap = false) { generate_struct(tstruct); }
/*
  void generate_struct_definition(t_struct* tstruct,
                                  bool setters = true);
*/
  void generate_struct_reader(t_struct* tstruct, bool pointers = false) {}
  void generate_struct_writer(t_struct* tstruct, bool pointers = false) {}
  void generate_struct_result_writer(t_struct* tstruct, bool pointers = false) {}
 private:
  ofstream f_mod_;
};

void t_rs_generator::init_generator() {
  // Make output directory
  MKDIR(get_out_dir().c_str());
  string pname = underscore(program_name_);
  string moddirname = get_out_dir() + pname + "/";
  MKDIR(moddirname.c_str());

  // Make output file
  string f_mod_name = moddirname + "mod.rs";
  f_mod_.open(f_mod_name.c_str());

  // Print header
  f_mod_ << rs_autogen_comment() << "\n";
  f_mod_ << rs_imports() << "\n";
}

void t_rs_generator::close_generator() {
  f_mod_.close();
}

string t_rs_generator::rs_autogen_comment() {
  return string(
    "///////////////////////////////////////////////////////////////\n") +
    "// Autogenerated by Thrift Compiler (" + THRIFT_VERSION + ")\n" +
    "//\n" +
    "// DO NOT EDIT UNLESS YOU ARE SURE YOU KNOW WHAT YOU ARE DOING\n" +
    "///////////////////////////////////////////////////////////////\n";
}

string t_rs_generator::rs_imports() {
  return string("#[allow(unused_imports)]\nuse std::collections::{HashMap, HashSet};\n");
}

void t_rs_generator::generate_typedef(t_typedef* ttypedef) {
  string tname = capitalize(ttypedef->get_symbolic());
  string tdef = render_rs_type(ttypedef->get_type());
  indent(f_mod_) << "pub type " << tname << " = " << tdef << ";\n";
  f_mod_ << "\n";
}

void t_rs_generator::generate_enum(t_enum* tenum) {
  string ename = capitalize(tenum->get_name());
  indent(f_mod_) << "pub enum " << ename << " {\n";
  indent_up();

  vector<t_enum_value*> constants = tenum->get_constants();
  vector<t_enum_value*>::iterator i, end = constants.end();
  for (i = constants.begin(); i != end; ++i) {
    string name = capitalize((*i)->get_name());
    int value = (*i)->get_value();
    indent(f_mod_) << name << " = " << value << ",\n";
  }

  indent_down();
  indent(f_mod_) << "}\n\n";
}

void t_rs_generator::generate_struct(t_struct* tstruct) {
  string struct_name = capitalize(tstruct->get_name());
  indent(f_mod_) << "#[allow(dead_code)]\n";
  if (tstruct->get_members().empty()) {
    indent(f_mod_) << "pub struct " << struct_name << ";\n\n";
  }
  else {
    indent(f_mod_) << "pub struct " << struct_name << " {\n";
    indent_up();

    vector<t_field*>::const_iterator m_iter;
    const vector<t_field*>& members = tstruct->get_members();
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
      t_type* t = get_true_type((*m_iter)->get_type());
      indent(f_mod_) << underscore((*m_iter)->get_name()) << ": " << render_rs_type(t) << ",\n";
    }

    indent_down();
    indent(f_mod_) << "}\n\n";
  }
}

void t_rs_generator::generate_service(t_service* tservice) {
  // TODO
//  generate_service_interface(tservice, "");
//  generate_service_interface_factory(tservice, "");
//  generate_service_null(tservice, "");

  generate_service_helpers(tservice);
  generate_service_client(tservice);
  // TODO

//  generate_service_processor(tservice, "");
//  generate_service_multiface(tservice);
//  generate_service_skeleton(tservice);
}

void t_rs_generator::generate_service_helpers(t_service* tservice) {
  // TODO
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;

  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    generate_function_helpers(tservice, *f_iter);
  }
}

void t_rs_generator::generate_service_client(t_service* tservice) {
  // TODO
}

/**
 * Generates a struct and helpers for a function.
 *
 * @param tfunction The function
 */
void t_rs_generator::generate_function_helpers(t_service* tservice, t_function* tfunction) {

  t_struct* ts = tfunction->get_arglist();
  string name_orig = ts->get_name();

  ts->set_name(tservice->get_name() + capitalize(tfunction->get_name()) + "Args");
  generate_struct(ts);
  //generate_struct_definition(out, f_service_, ts, false);
  generate_struct_reader(ts);
  generate_struct_writer(ts);

  ts->set_name(tservice->get_name() + capitalize(tfunction->get_name()) + "PArgs");
  generate_struct_declaration(ts, false, true, false, true);
  //generate_struct_definition(out, f_service_, ts, false);
  generate_struct_writer(ts, true);

  ts->set_name(name_orig);

  if (tfunction->is_oneway()) {
    return;
  }

  t_struct result(program_, tservice->get_name() + capitalize(tfunction->get_name()) + "Result");
  t_field success(tfunction->get_returntype(), "success", 0);
  if (!tfunction->get_returntype()->is_void()) {
    result.append(&success);
  }

  t_struct* xs = tfunction->get_xceptions();
  const vector<t_field*>& fields = xs->get_members();
  vector<t_field*>::const_iterator f_iter;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    result.append(*f_iter);
  }

  generate_struct_declaration(&result, false);
  //generate_struct_definition(f_service_, &result, false);
  generate_struct_reader(&result);
  generate_struct_result_writer(&result);

  result.set_name(tservice->get_name() + capitalize(tfunction->get_name()) + "PResult");
  generate_struct_declaration(&result, false, true, true /*, gen_cob_style_*/);
  //generate_struct_definition(f_service_, &result, false);
  generate_struct_reader(&result, true);
/*
  if (gen_cob_style_) {
    generate_struct_writer(&result, true);
  }
*/
}


string t_rs_generator::render_rs_type(t_type* type) {
  type = get_true_type(type);

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
    case t_base_type::TYPE_VOID:
      return "()";
    case t_base_type::TYPE_STRING:
      return (((t_base_type*)type)->is_binary() ? "Vec<u8>" : "String");
    case t_base_type::TYPE_BOOL:
      return "bool";
    case t_base_type::TYPE_BYTE:
      return "i8";
    case t_base_type::TYPE_I16:
      return "i16";
    case t_base_type::TYPE_I32:
      return "i32";
    case t_base_type::TYPE_I64:
      return "i64";
    case t_base_type::TYPE_DOUBLE:
      return "f64";
    }

  } else if (type->is_enum()) {
    return capitalize(((t_enum*)type)->get_name());

  } else if (type->is_struct() || type->is_xception()) {
    return capitalize(((t_struct*)type)->get_name());

  } else if (type->is_map()) {
    t_type* ktype = ((t_map*)type)->get_key_type();
    t_type* vtype = ((t_map*)type)->get_val_type();
    return "HashMap<" + render_rs_type(ktype) + ", " + render_rs_type(vtype) + ">";

  } else if (type->is_set()) {
    t_type* etype = ((t_set*)type)->get_elem_type();
    return "HashSet<" + render_rs_type(etype) + ">";

  } else if (type->is_list()) {
    t_type* etype = ((t_list*)type)->get_elem_type();
    return "Vec<" + render_rs_type(etype) + ">";

  } else {
    throw "INVALID TYPE IN type_to_enum: " + type->get_name();
  }
}

THRIFT_REGISTER_GENERATOR(rs, "Rust", "")
