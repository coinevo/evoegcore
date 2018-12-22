#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../src/UniversalIdentifier.hpp"

#include "mocks.h"

#define ADD_MOCKS(mcore) \
    EXPECT_CALL(mcore, get_output_tx_and_index(_, _, _)) \
            .WillRepeatedly( \
                Invoke(&*jtx, &JsonTx::get_output_tx_and_index)); \
        \
    EXPECT_CALL(mcore, get_tx(_, _)) \
            .WillRepeatedly( \
                Invoke(&*jtx, &JsonTx::get_tx)); \
        \
    EXPECT_CALL(mcore, get_output_key(_, _, _)) \
            .WillRepeatedly( \
                Invoke(&*jtx, &JsonTx::get_output_key));


namespace
{

using namespace xmreg;


// equality operators for outputs

inline bool
operator==(const Output::info& lhs, const JsonTx::output& rhs)
{
    return lhs.amount == rhs.amount
            && lhs.pub_key == rhs.pub_key
            && lhs.idx_in_tx == rhs.index;
}

inline bool
operator!=(const Output::info& lhs, const JsonTx::output& rhs)
{
    return !(lhs == rhs);
}

inline bool
operator==(const vector<Output::info>& lhs, const vector<JsonTx::output>& rhs)
{
    if (lhs.size() != rhs.size())
        return false;

    for (size_t i = 0; i < lhs.size(); i++)
    {
        if (lhs[i] != rhs[i])
            return false;
    }

    return true;
}



// equality operators for inputs

inline bool
operator==(const Input::info& lhs, const JsonTx::input& rhs)
{
    return lhs.amount == rhs.amount
            && lhs.out_pub_key == rhs.out_pub_key
            && lhs.key_img == rhs.key_img;
}

inline bool
operator!=(const Input::info& lhs, const JsonTx::input& rhs)
{
    return !(lhs == rhs);
}

inline bool
operator==(const vector<Input::info>& lhs, const vector<JsonTx::input>& rhs)
{
    if (lhs.size() != rhs.size())
        return false;

    for (size_t i = 0; i < lhs.size(); i++)
    {
        if (lhs[i] != rhs[i])
            return false;
    }

    return true;
}

class DifferentJsonTxs :
        public ::testing::TestWithParam<string>
{

};

class ModularIdentifierTest : public DifferentJsonTxs
{};

INSTANTIATE_TEST_CASE_P(
    DifferentJsonTxs, ModularIdentifierTest,
    ::testing::Values(
        "ddff95211b53c194a16c2b8f37ae44b643b8bd46b4cb402af961ecabeb8417b2"s,
        "f3c84fe925292ec5b4dc383d306d934214f4819611566051bca904d1cf4efceb"s,
        "d7dcb2daa64b5718dad71778112d48ad62f4d5f54337037c420cb76efdd8a21c"s));

TEST_P(ModularIdentifierTest, OutputsRingCT)
{
    string tx_hash_str = GetParam();

    auto jtx = construct_jsontx(tx_hash_str);

    ASSERT_TRUE(jtx);

    auto identifier = make_identifier(jtx->tx,
          make_unique<Output>(&jtx->sender.address,
                              &jtx->sender.viewkey));

    identifier.identify();

    ASSERT_EQ(identifier.get<0>()->get().size(),
              jtx->sender.outputs.size());

    ASSERT_TRUE(identifier.get<0>()->get() == jtx->sender.outputs);

    ASSERT_EQ(identifier.get<0>()->get_total(),
              jtx->sender.change);
}


TEST_P(ModularIdentifierTest, OutputsRingCTCoinbaseTx)
{
    string tx_hash_str = GetParam();

    auto jtx = construct_jsontx(tx_hash_str);

    ASSERT_TRUE(jtx);

    auto identifier = make_identifier(jtx->tx,
          make_unique<Output>(&jtx->recipients.at(0).address,
                              &jtx->recipients.at(0).viewkey));

    identifier.identify();

    ASSERT_TRUE(identifier.get<0>()->get()
                == jtx->recipients.at(0).outputs);

    ASSERT_EQ(identifier.get<0>()->get_total(),
              jtx->recipients.at(0).amount);
}

TEST_P(ModularIdentifierTest, MultiOutputsRingCT)
{
    string tx_hash_str = GetParam();

    auto jtx = construct_jsontx(tx_hash_str);

    ASSERT_TRUE(jtx);

    for (auto const& jrecipient: jtx->recipients)
    {
        auto identifier = make_identifier(jtx->tx,
              make_unique<Output>(&jrecipient.address,
                                  &jrecipient.viewkey));

       identifier.identify();

       EXPECT_TRUE(identifier.get<0>()->get()
                    == jrecipient.outputs);
    }
}


TEST(MODULAR_IDENTIFIER, LegacyPaymentID)
{
    auto jtx = construct_jsontx("d7dcb2daa64b5718dad71778112d48ad62f4d5f54337037c420cb76efdd8a21c");

    ASSERT_TRUE(jtx);

    auto identifier = make_identifier(jtx->tx,
          make_unique<LegacyPaymentID>(nullptr, nullptr));

   identifier.identify();

   EXPECT_TRUE(identifier.get<0>()->get()
                == jtx->payment_id);

   EXPECT_TRUE(identifier.get<0>()->raw()
                == jtx->payment_id);
}

TEST(MODULAR_IDENTIFIER, IntegratedPaymentID)
{
    auto jtx = construct_jsontx("ddff95211b53c194a16c2b8f37ae44b643b8bd46b4cb402af961ecabeb8417b2");

    ASSERT_TRUE(jtx);

    auto identifier = make_identifier(jtx->tx,
          make_unique<IntegratedPaymentID>(
                         &jtx->recipients[0].address,
                         &jtx->recipients[0].viewkey));

   identifier.identify();

   EXPECT_TRUE(identifier.get<0>()->get()
                == jtx->payment_id8e);

   EXPECT_TRUE(identifier.get<0>()->raw()
                == jtx->payment_id8);
}


TEST_P(ModularIdentifierTest, GuessInputRingCT)
{
    string tx_hash_str = GetParam();

    auto jtx = construct_jsontx(tx_hash_str);

    ASSERT_TRUE(jtx);

    MockMicroCore mcore;

    ADD_MOCKS(mcore);

    auto identifier = make_identifier(jtx->tx,
          make_unique<GuessInput>(
                    &jtx->sender.address,
                    &jtx->sender.viewkey,
                    &mcore));

   identifier.identify();

//   for (auto const& input_info: identifier.get<0>()->get())
//       cout << input_info << endl;

   EXPECT_TRUE(identifier.get<0>()->get()
                == jtx->sender.inputs);
}

TEST_P(ModularIdentifierTest, RealInputRingCT)
{
    string tx_hash_str = GetParam();

    auto jtx = construct_jsontx(tx_hash_str);

    ASSERT_TRUE(jtx);

    MockMicroCore mcore;

    ADD_MOCKS(mcore);

    auto identifier = make_identifier(jtx->tx,
          make_unique<RealInput>(
                    &jtx->sender.address,
                    &jtx->sender.viewkey,
                    &jtx->sender.spendkey,
                    &mcore));

   identifier.identify();

//   for (auto const& input_info: identifier.get<0>()->get())
//       cout << input_info << endl;

   EXPECT_TRUE(identifier.get<0>()->get()
                == jtx->sender.inputs);
}

}
