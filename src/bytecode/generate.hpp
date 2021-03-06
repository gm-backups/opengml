#pragma once

#include "ogm/bytecode/bytecode.hpp"
#include "ogm/bytecode/Library.hpp"
#include "ogm/bytecode/Namespace.hpp"
#include "ogm/bytecode/stream_macro.hpp"

#include "ogm/asset/AssetTable.hpp"

#include "ogm/common/error.hpp"
#include "ogm/common/util.hpp"

#include <cstring>
#include <map>
#include <string>
#include <cassert>

namespace ogm::bytecode
{

typedef int32_t bytecode_address_t;
constexpr bytecode_address_t k_invalid_pos = -1;
constexpr bytecode_address_t k_placeholder_pos = -2;

struct GenerateContextArgs
{
    GenerateContextArgs(uint8_t retc, uint8_t argc, ProjectAccumulator* accumulator, const DecoratedAST* dast, bytecode_index_t bytecode_index, Namespace& instance, Namespace& globals, const Library* library, const asset::AssetTable* asset_table, const bytecode::BytecodeTable* bytecode_table, std::vector<bytecode_address_t>& break_placeholder_vector, std::vector<bytecode_address_t>& continue_placeholder_vector, std::vector<opcode::opcode_t>& cleanup_commands, DebugSymbols* symbols, ReflectionAccumulator* reflection, GenerateConfig* config, bytecode_address_t& peephole_horizon)
        : m_retc(retc)
        , m_argc(argc)
        , m_accumulator(accumulator)
        , m_dast(dast)
        , m_bytecode_index(bytecode_index)
        , m_instance_variables(instance)
        , m_globals(globals)
        , m_library(library)
        , m_asset_table(asset_table)
        , m_bytecode_table(bytecode_table)
        , m_break_placeholders(break_placeholder_vector)
        , m_continue_placeholders(continue_placeholder_vector)
        , m_cleanup_commands(cleanup_commands)
        , m_continue_pop_count(0)
        , m_enum_expression(false)
        , m_symbols(symbols)
        , m_reflection(reflection)
        , m_config(config)
        , m_peephole_horizon(peephole_horizon)
        #ifdef OGM_FUNCTION_SUPPORT
        , m_lambda_id{ new int32_t() }
        #endif
        , m_statics{ new std::map<std::string, variable_id_t>() }
    { }

    GenerateContextArgs(const GenerateContextArgs& other)
        : m_retc(other.m_retc)
        , m_argc(other.m_argc)
        , m_accumulator(other.m_accumulator)
        , m_dast(other.m_dast)
        , m_bytecode_index(other.m_bytecode_index)
        , m_instance_variables(other.m_instance_variables)
        , m_globals(other.m_globals)
        , m_library(other.m_library)
        , m_asset_table(other.m_asset_table)
        , m_bytecode_table(other.m_bytecode_table)
        , m_break_placeholders(other.m_break_placeholders)
        , m_continue_placeholders(other.m_continue_placeholders)
        , m_cleanup_commands(other.m_cleanup_commands)
        , m_continue_pop_count(other.m_continue_pop_count)
        , m_enum_expression(other.m_enum_expression)
        , m_symbols(other.m_symbols)
        , m_reflection(other.m_reflection)
        , m_config(other.m_config)
        , m_peephole_horizon(other.m_peephole_horizon)
        #ifdef OGM_FUNCTION_SUPPORT
        , m_lambda_id(other.m_lambda_id)
        #endif
        , m_statics(other.m_statics)
    { }

    uint8_t m_retc, m_argc;
    
    ProjectAccumulator* m_accumulator;
    
    const DecoratedAST* m_dast;
    
    bytecode_index_t m_bytecode_index;

    // TODO: many of these are no longer needed since we have a ProjectAccumulator.
    Namespace& m_instance_variables;
    Namespace& m_globals;

    const Library* m_library;
    const asset::AssetTable* m_asset_table;
    const bytecode::BytecodeTable* m_bytecode_table;

    // addresses which should later be filled in with the break address.
    std::vector<bytecode_address_t>& m_break_placeholders;

    // addresses which should later be filled in with the continue address.
    std::vector<bytecode_address_t>& m_continue_placeholders;

    // the sequence of cleanup commands required before returning.
    std::vector<opcode::opcode_t>& m_cleanup_commands;

    // number of 'pop' statements required to continue (important for switch statements)
    uint16_t m_continue_pop_count;
    bool m_enum_expression;

    DebugSymbols* m_symbols;

    ReflectionAccumulator* m_reflection;

    GenerateConfig* m_config;
    
    bytecode_address_t& m_peephole_horizon;
    
    #ifdef OGM_FUNCTION_SUPPORT
    // just used for the name of anonymous function literals
    std::shared_ptr<int32_t> m_lambda_id;
    #endif
    
    // static variable map
    std::shared_ptr<std::map<std::string, variable_id_t>> m_statics;
};

// prevents peephole optimizations modifying before this point.
void peephole_block(std::ostream& out, GenerateContextArgs& context_args);

// optimize the recent bytecode.
void peephole_optimize(std::ostream& out, GenerateContextArgs& context_args);

void bytecode_generate_ast(std::ostream& out, const ogm_ast_t& ast, GenerateContextArgs context_args);

}