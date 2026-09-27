#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

// --------------------------------------------------------------------------
// Current money for this editor session. Lives in memory; optionally mirrored
// to saved mod data if "persist-money" is enabled (see LevelEditorLayer hook).
// --------------------------------------------------------------------------
static int g_money = 0;

// --------------------------------------------------------------------------
// Price table. Key = GD object ID, value = price in dollars.
// Add/adjust entries here for whatever balance you want.
// Anything not listed falls back to the "default-object-price" setting.
// A handful of common IDs are seeded below as an example / starting point.
// --------------------------------------------------------------------------
static const std::unordered_map<int, int>& getPriceTable() {
    static const std::unordered_map<int, int> prices = {
        {1, 1},     // basic block
        {2, 1},     // basic block variant
        {8, 5},     // spike
        {9, 5},     // spike variant
        {103, 50},  // yellow jump orb
        {141, 50},  // pink jump orb
        {1704, 75}, // blue jump orb
        {1751, 75}, // red jump orb
        {99, 40},   // yellow jump pad
        {10, 3},    // slope block
        {11, 3},    // slope block variant
    };
    return prices;
}

static int getPriceFor(int objectID) {
    auto const& table = getPriceTable();
    auto it = table.find(objectID);
    if (it != table.end()) {
        return it->second;
    }
    return static_cast<int>(Mod::get()->getSettingValue<int64_t>("default-object-price"));
}

// --------------------------------------------------------------------------
// EditorUI: draws the money HUD and gates object placement on funds.
// --------------------------------------------------------------------------
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

    // This is the hook that actually creates & places a new object in the
    // level. Blocking here (by not calling the original / returning nullptr)
    // stops the object from ever being placed.
    GameObject* createObject(int type, CCPoint pos, bool useObjectPos) {
        int price = getPriceFor(type);

        if (g_money < price) {
            Notification::create(
                fmt::format("Not enough money! Need $ {}", price),
                NotificationIcon::Error
            )->show();
            return nullptr;
        }

        auto obj = EditorUI::createObject(type, pos, useObjectPos);

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

// --------------------------------------------------------------------------
// LevelEditorLayer: sets up the starting balance whenever the editor opens.
// --------------------------------------------------------------------------
class $modify(MoneyLevelEditorLayer, LevelEditorLayer) {
    bool init(GJGameLevel* level) {
        if (!LevelEditorLayer::init(level)) return false;

        bool persist = Mod::get()->getSettingValue<bool>("persist-money");

        if (persist) {
            // Carry over money from the last session, or fall back to the
            // configured starting amount the very first time.
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
