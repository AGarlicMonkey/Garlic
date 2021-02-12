#include <Clove/Serialisation/Node.hpp>
#include <Clove/Serialisation/Yaml.hpp>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace garlic::clove;
using namespace garlic::clove::serialiser;

class YamlDeserialisationTests : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        {
            std::ofstream testFile{ "TestFile.yaml" };
            testFile << "type: yaml" << std::endl;
            testFile << "version: 1" << std::endl;
            testFile << "IntValue: 3" << std::endl;
            testFile << "FloatValue: 4.5" << std::endl;
            testFile << "StringValue: \"Hello, World!\"" << std::endl;
            testFile << "Parent:" << std::endl;
            testFile << "  ChildOne: 1" << std::endl;
            testFile << "  ChildTwo:" << std::endl;
            testFile << "    Value: 8" << std::endl;
            testFile << "TestType:" << std::endl;
            testFile << "  memberOne: 420" << std::endl;
            testFile << "  memberTwo: 1.5" << std::endl;
            testFile << "TestParentType:" << std::endl;
            testFile << "  memberOne:" << std::endl;
            testFile << "    memberOne: 100" << std::endl;
            testFile << "    memberTwo: 200" << std::endl;
            testFile << "  memberTwo: 99" << std::endl;
        }

        {
            std::ofstream wrongType{ "WrongType.yaml" };
            wrongType << "type: asodiufg" << std::endl;
        }

        {
            std::ofstream wrongVersion{ "WrongVersion.yaml" };
            wrongVersion << "type: yaml" << std::endl;
            wrongVersion << "version: -1" << std::endl;
        }
    }

    static void TearDownTestSuite() {
        std::remove("TestFile.yaml");
        std::remove("WrongType.yaml");
        std::remove("WrongVersion.yaml");
    }
};

TEST_F(YamlDeserialisationTests, CanLoadSimpleValueFromFile) {
    Node file{ *loadYaml("TestFile.yaml") };

    EXPECT_EQ(file["IntValue"].as<int32_t>(), 3);
    EXPECT_EQ(file["FloatValue"].as<float>(), 4.5f);
    EXPECT_EQ(file["StringValue"].as<std::string>(), "Hello, World!");
}

TEST_F(YamlDeserialisationTests, CanLoadNestedValuesFromParentNodes) {
    Node file{ *loadYaml("TestFile.yaml") };

    EXPECT_EQ(file["Parent"]["ChildOne"].as<int32_t>(), 1);
    EXPECT_EQ(file["Parent"]["ChildTwo"]["Value"].as<int32_t>(), 8);
}

TEST_F(YamlDeserialisationTests, CannotGetIncorrectType) {
    Node file{ *loadYaml("TestFile.yaml") };

    EXPECT_ANY_THROW(file["TestType"].as<std::string>());
}

TEST_F(YamlDeserialisationTests, ErrorsWhenOpeningNoFile) {
    Expected<Node, LoadError> file{ loadYaml("ThisShouldNeverExist.yaml") };

    ASSERT_FALSE(file.hasValue());
    EXPECT_EQ(file.getError(), LoadError::BadFile);
}

TEST_F(YamlDeserialisationTests, CannotLoadIncorrectSerialisedType) {
    Expected<Node, LoadError> file{ loadYaml("WrongType.yaml") };

    ASSERT_FALSE(file.hasValue());
    EXPECT_EQ(file.getError(), LoadError::WrongType);
}

TEST_F(YamlDeserialisationTests, CannotLoadIncorrectSerialisedVersion) {
    Expected<Node, LoadError> file{ loadYaml("WrongVersion.yaml") };

    ASSERT_FALSE(file.hasValue());
    EXPECT_EQ(file.getError(), LoadError::WrongVersion);
}

struct BasicSerialisableType {
    int32_t memberOne;
    float memberTwo;
};

namespace garlic::clove {
    template<>
    Node serialise<BasicSerialisableType>(BasicSerialisableType const &object) {
        Node node{};
        node["memberOne"] = object.memberOne;
        node["memberTwo"] = object.memberTwo;
        return node;
    }

    template<>
    BasicSerialisableType deserialise(Node const &node) {
        return BasicSerialisableType{
            .memberOne = node["memberOne"].as<int32_t>(),
            .memberTwo = node["memberTwo"].as<float>(),
        };
    }
}

TEST_F(YamlDeserialisationTests, CanLoadSerialisableTypeFromFile) {
    Node file{ *loadYaml("TestFile.yaml") };

    auto type{ file["TestType"].as<BasicSerialisableType>() };

    EXPECT_EQ(type.memberOne, 420);
    EXPECT_EQ(type.memberTwo, 1.5f);
}

struct ParentSerialisableType {
    BasicSerialisableType memberOne;
    int32_t memberTwo;
};

namespace garlic::clove {
    template<>
    Node serialise<ParentSerialisableType>(ParentSerialisableType const &object) {
        Node node{};
        node["memberOne"] = object.memberOne;
        node["memberTwo"] = object.memberTwo;
        return node;
    }

    template<>
    ParentSerialisableType deserialise(Node const &node) {
        return ParentSerialisableType{
            .memberOne = node["memberOne"].as<BasicSerialisableType>(),
            .memberTwo = node["memberTwo"].as<int32_t>(),
        };
    }
}

TEST_F(YamlDeserialisationTests, CanLoadSerialisableParentTypeFromFile) {
    Node file{ *loadYaml("TestFile.yaml") };

    auto type{ file["TestParentType"].as<ParentSerialisableType>() };

    EXPECT_EQ(type.memberOne.memberOne, 100);
    EXPECT_EQ(type.memberOne.memberTwo, 200);
    EXPECT_EQ(type.memberTwo, 99);
}