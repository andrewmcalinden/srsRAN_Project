/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "lib/scheduler/support/pusch/pusch_dmrs_symbol_mask.h"
#include "srsgnb/adt/static_vector.h"
#include "srsgnb/srslog/bundled/fmt/ostream.h"
#include <gtest/gtest.h>
#include <map>

using namespace srsgnb;

namespace srsgnb {

std::ostream& operator<<(std::ostream& os, dmrs_typeA_position dmrs_pos)
{
  switch (dmrs_pos) {
    case dmrs_typeA_position::pos2:
      return os << "pos2";
    case dmrs_typeA_position::pos3:
    default:
      return os << "pos3";
  }
}

std::ostream& operator<<(std::ostream& os, dmrs_additional_positions additional_pos)
{
  switch (additional_pos) {
    case dmrs_additional_positions::pos0:
      return os << "pos0";
    case dmrs_additional_positions::pos1:
      return os << "pos1";
    case dmrs_additional_positions::pos2:
      return os << "pos2";
    case dmrs_additional_positions::pos3:
    default:
      return os << "pos3";
  }
}

std::ostream& operator<<(std::ostream& os, dmrs_symbol_mask mask)
{
  fmt::print(os, "{}", mask);
  return os;
}

} // namespace srsgnb

namespace {
#define L0 (UINT8_MAX)

using PuschDmrsSymbolMaskParams = std::tuple<dmrs_typeA_position, unsigned, dmrs_additional_positions>;

class PuschDmrsSymbolMaskFixture : public ::testing::TestWithParam<PuschDmrsSymbolMaskParams>
{};

const std::map<std::tuple<unsigned, dmrs_additional_positions>, static_vector<unsigned, 4>>
    pusch_dmrs_symbol_mask_typeA_single_table = {
        {{4, dmrs_additional_positions::pos0}, {L0}},         {{4, dmrs_additional_positions::pos1}, {L0}},
        {{4, dmrs_additional_positions::pos2}, {L0}},         {{4, dmrs_additional_positions::pos3}, {L0}},
        {{5, dmrs_additional_positions::pos0}, {L0}},         {{5, dmrs_additional_positions::pos1}, {L0}},
        {{5, dmrs_additional_positions::pos2}, {L0}},         {{5, dmrs_additional_positions::pos3}, {L0}},
        {{6, dmrs_additional_positions::pos0}, {L0}},         {{6, dmrs_additional_positions::pos1}, {L0}},
        {{6, dmrs_additional_positions::pos2}, {L0}},         {{6, dmrs_additional_positions::pos3}, {L0}},
        {{7, dmrs_additional_positions::pos0}, {L0}},         {{7, dmrs_additional_positions::pos1}, {L0}},
        {{7, dmrs_additional_positions::pos2}, {L0}},         {{7, dmrs_additional_positions::pos3}, {L0}},
        {{8, dmrs_additional_positions::pos0}, {L0}},         {{8, dmrs_additional_positions::pos1}, {L0, 7}},
        {{8, dmrs_additional_positions::pos2}, {L0, 7}},      {{8, dmrs_additional_positions::pos3}, {L0, 7}},
        {{9, dmrs_additional_positions::pos0}, {L0}},         {{9, dmrs_additional_positions::pos1}, {L0, 7}},
        {{9, dmrs_additional_positions::pos2}, {L0, 7}},      {{9, dmrs_additional_positions::pos3}, {L0, 7}},
        {{10, dmrs_additional_positions::pos0}, {L0}},        {{10, dmrs_additional_positions::pos1}, {L0, 9}},
        {{10, dmrs_additional_positions::pos2}, {L0, 6, 9}},  {{10, dmrs_additional_positions::pos3}, {L0, 6, 9}},
        {{11, dmrs_additional_positions::pos0}, {L0}},        {{11, dmrs_additional_positions::pos1}, {L0, 9}},
        {{11, dmrs_additional_positions::pos2}, {L0, 6, 9}},  {{11, dmrs_additional_positions::pos3}, {L0, 6, 9}},
        {{12, dmrs_additional_positions::pos0}, {L0}},        {{12, dmrs_additional_positions::pos1}, {L0, 9}},
        {{12, dmrs_additional_positions::pos2}, {L0, 6, 9}},  {{12, dmrs_additional_positions::pos3}, {L0, 5, 8, 11}},
        {{13, dmrs_additional_positions::pos0}, {L0}},        {{13, dmrs_additional_positions::pos1}, {L0, 11}},
        {{13, dmrs_additional_positions::pos2}, {L0, 7, 11}}, {{13, dmrs_additional_positions::pos3}, {L0, 5, 8, 11}},
        {{14, dmrs_additional_positions::pos0}, {L0}},        {{14, dmrs_additional_positions::pos1}, {L0, 11}},
        {{14, dmrs_additional_positions::pos2}, {L0, 7, 11}}, {{14, dmrs_additional_positions::pos3}, {L0, 5, 8, 11}}};

TEST_P(PuschDmrsSymbolMaskFixture, ANormal)
{
  // Extract parameters.
  dmrs_typeA_position       typeA_pos           = std::get<0>(GetParam());
  unsigned                  duration            = std::get<1>(GetParam());
  dmrs_additional_positions additional_position = std::get<2>(GetParam());

  // Prepare configuration.
  pusch_dmrs_symbol_mask_mapping_type_A_single_configuration config;
  config.typeA_pos           = typeA_pos;
  config.duration            = duration;
  config.additional_position = static_cast<dmrs_additional_positions>(additional_position);

  // Get mask.
  dmrs_symbol_mask mask = pusch_dmrs_symbol_mask_mapping_type_A_single_get(config);

  // Get expected symbol position list from table.
  static_vector<unsigned, 4> symbol_pos_list =
      pusch_dmrs_symbol_mask_typeA_single_table.at({duration, additional_position});

  // Convert list to mask.
  dmrs_symbol_mask expected(14);
  ASSERT_TRUE(pusch_dmrs_symbol_mask_typeA_single_table.count({duration, additional_position}));
  for (unsigned symbol_index : pusch_dmrs_symbol_mask_typeA_single_table.at({duration, additional_position})) {
    if (symbol_index == L0) {
      symbol_index = to_symbol_index(typeA_pos);
    }
    expected.set(symbol_index);
  }

  // Assert each mask with the expected.
  ASSERT_EQ(mask, expected);
}

INSTANTIATE_TEST_SUITE_P(PuschDmrsSymbolMaskTypeASingle,
                         PuschDmrsSymbolMaskFixture,
                         ::testing::Combine(::testing::Values(dmrs_typeA_position::pos2, dmrs_typeA_position::pos3),
                                            ::testing::Range(4U, 14U),
                                            ::testing::Values(dmrs_additional_positions::pos0,
                                                              dmrs_additional_positions::pos1,
                                                              dmrs_additional_positions::pos2,
                                                              dmrs_additional_positions::pos3)));

} // namespace