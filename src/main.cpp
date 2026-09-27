#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

static int g_money = 0;

static const std::unordered_map<int, int>& getGameplayPriceTable() {
    static const std::unordered_map<int, int> prices = {
        {1, 1}, {2, 1}, {8, 5}, {9, 5}, {103, 50},
        {141, 50}, {1704, 75}, {1751, 75}, {99, 40}, {10, 3}, {11, 3},
    };
    return prices;
}

enum class DecorationTier { Simple, Medium, Complex, Animated };

static int tierPrice(DecorationTier tier) {
    switch (tier) {
        case DecorationTier::Simple:   return 2;
        case DecorationTier::Medium:   return 8;
        case DecorationTier::Complex:  return 25;
        case DecorationTier::Animated: return 40;
    }
    return 2;
}

static const std::unordered_map<int, DecorationTier>& getDecorationTierTable() {
    static const std::unordered_map<int, DecorationTier> tiers = {
        // Fill these in with real object IDs from your own editor, e.g.:
        // {462, DecorationTier::Simple},
        // {174, DecorationTier::Medium},
        // {493, DecorationTier::Complex},
        // {1330, DecorationTier::Animated},
    };
    return tiers;
}

static int getPriceFor(int objectID) {
    auto const& gameplay = getGameplayPriceTable();
    auto git = gameplay.find(objectID);
    if (git != gameplay.end()) return git->second;

    auto const& decorations = getDecorationTierTable();
    auto dit = decorations.find(objectID);
    if (dit != decorations.end()) return tierPrice(dit->second);

    return static_cast<int>(Mod::get()->getSettingValue<int64_t>("default-object-price"));
}

class $modify(MoneyEditorUI, EditorUI) {
    struct Fields {
        CCLabelBMFont* moneyLabel = nullptr;
    };

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto label = CCLabelBMFont::create(this->moneyText().c_str(), "bigFont.fnt");
        label->setScale(0.5f);
        label->setAnchorPoint({0.f, 1.f});
        label->setPosition({10.f, winSize.height - 10.f});
        label->setID("money-label"_spr);
        label->setZOrder(1000);
        label->setColor({255, 255, 0});
        this->addChild(label);
        m_fields->moneyLabel = label;
        return true;
    }

    std::string moneyText() {
        return fmt::format("$ {}", g_money);
    }

    void refreshMoneyLabel() {
        if (m_fields->moneyLabel) {
            m_fields->moneyLabel->setString(this->moneyText().c_str());
        }
    }

    // This GD/Geode version's createObject takes only (objectID, position) -
    // no third bool parameter.
    GameObject* createObject(int type, CCPoint pos) {
        int price = getPriceFor(type);
        if (g_money < price) {
            Notification::create(
                fmt::format("Not enough money! Need $ {}", price),
                NotificationIcon::Error
            )->show();
            return nullptr;
        }
        auto obj = EditorUI::createObject(type, pos);
        if (obj) {
            g_money -= price;
            if (Mod::get()->getSettingValue<bool>("persist-money")) {
                Mod::get()->setSavedValue("saved-money", g_money);
            }
            this->refreshMoneyLabel();
        }
        return obj;
    }
};

class $modify(MoneyLevelEditorLayer, LevelEditorLayer) {
    // This GD/Geode version's init takes (level, noUI) - pass false for noUI
    // (normal editor entry, not the reduced "no UI" mode).
    bool init(GJGameLevel* level, bool noUI) {
        if (!LevelEditorLayer::init(level, noUI)) return false;
        bool persist = Mod::get()->getSettingValue<bool>("persist-money");
        if (persist) {
            g_money = Mod::get()->getSavedValue<int>(
                "saved-money",
                static_cast<int>(Mod::get()->getSettingValue<int64_t>("starting-money"))
            );
        } else {
            g_money = static_cast<int>(Mod::get()->getSettingValue<int64_t>("starting-money"));
        }
        return true;
    }
};
