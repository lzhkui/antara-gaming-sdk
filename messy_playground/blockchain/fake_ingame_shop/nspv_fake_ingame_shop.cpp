//
// Created by Roman Szterg on 25/11/2019.
//

#include <imgui.h>

#include <thread>
#include <mutex>
#include <chrono>

#include <antara/gaming/world/world.app.hpp>
#include <antara/gaming/sfml/graphic.system.hpp>
#include <antara/gaming/sfml/resources.manager.hpp>
#include <antara/gaming/sfml/input.system.hpp>
#include <antara/gaming/blockchain/nspv.system.hpp>
#include <iostream>
#include <sstream>
#include <antara/gaming/core/open.url.browser.hpp>

using namespace antara::gaming;
using namespace std::chrono_literals;

namespace {
    static const std::string currency{"RICK"};
    static const std::string currency_lc{"rick"};
    static const std::string explorer_url{"explorer.dexstats.info"};

    template<typename T>
    static std::string double_to_str(const T a_value, const int n = 2) {
        std::ostringstream out;
        out.precision(n);
        out << std::fixed << a_value;
        return out.str();
    }
}

class fake_shop final : public ecs::post_update_system<fake_shop> {
    struct item {
        int id;
        int quantity;
        double price;
        std::string name;
        std::string description;
    };

    struct inventory {
        double balance{0};
        double pending_balance{0};
        double nspv_balance{0};
        std::unordered_map<int, item> items;
        std::string wallet_address;
    };

public:
    void update_balances() {
        double s_balance = nspv_system_store_.get_balance(currency);
        double u_balance = nspv_system_user_.get_balance(currency);

        {
            std::scoped_lock st_lock(store_mutex, user_mutex);

            store.nspv_balance = s_balance;
            user.nspv_balance = u_balance;

            // Safe to update local balance if there are no pending transactions
            if (pending_transaction_count == 0 && mempool_transaction_count == 0) {
                store.balance = store.nspv_balance;
                user.balance = user.nspv_balance;
            }
        }
    }

    fake_shop(entt::registry &registry, blockchain::nspv &nspv_system_user) noexcept : system(registry),
                                                                                       nspv_system_user_(
                                                                                               nspv_system_user) {
        // Spawn nSPV for shop
        {
            nspv_system_store_.spawn_nspv_instance(currency, false, 7777);
            nspv_system_store_.load_from_env(currency, "SHOP_WIF_WALLET");
        }

        // Set wallet addresses
        {
            store.wallet_address = nspv_system_store_.get_address(currency);
            user.wallet_address = nspv_system_user_.get_address(currency);
        }

        // Set balances at start
        {
            update_balances();
        }

        // Refresh mempool transaction count and update balances accordingly
        {
            nspv_threads.emplace_back([this] {
                while (!application_quits) {
                    std::this_thread::sleep_for(5s);
                    // Update pending transactions count
                    blockchain::nspv_api::mempool_request request{user.wallet_address};

                    int m_tx_count = blockchain::nspv_api::mempool(nspv_system_user_.get_endpoint(currency),
                                                                   request).txids.size();
                    {
                        std::scoped_lock st_lock(user_mutex);
                        mempool_transaction_count = m_tx_count;
                    }

                    // Update balances
                    update_balances();
                }
            });
        }

        // Fill store
        {
            int id = -1;
            ++id;
            store.items[id] = {id, 5, 1, "Roma",
                               "Sometimes nothing beats a classic. We make ours the same way the original Italian masterpiece was prepared for Queen Margherita. A simple classic pie layered with fresh made whole milk mozzarella and finished with cold pressed olive oil fruttato drizzle and organic basil leaf chiffonade."};
            ++id;
            store.items[id] = {id, 5, 2, "5-Points",
                               "Named after one of the oldest neighborhoods in Denver, The 5-Points is our homage to the diversity of this historic district. We use our house red sauce and layer this cheese pizza with 5 delectable varieties. Mozzarella, provolone, muenster, cheddar and creamy ricotta impastata cheese, finished with fresh basil chiffonade."};
            ++id;
            store.items[id] = {id, 5, 2, "The Boulder",
                               "Our classic Colorado Pizza done veggie style, with green peppers, red onion, white button mushrooms, black olives and roma tomato."};
            ++id;
            store.items[id] = {id, 5, 2, "Twin Sisters",
                               "Named after the Colorado peaks of the same name, this pie is a classic beauty. We layer a pizza with house cut premium imported Italian pepperoni and imported soppressata salami."};
            ++id;
            store.items[id] = {id, 5, 3, "Durango",
                               "We take our classic pie and layer mozzarella and provolone and then add a wonderful garlic and fennel Italian sausage and white button mushrooms."};
            ++id;
            store.items[id] = {id, 5, 3, "The Baby Doe",
                               "Baby Doe Tabor was a 1st generation Irish American and one of Colorado's most colorful historic figures. A true \"Rags to Riches...back to Rags\" kind of gal. As such we created a pizza that showed her humble roots as well as her panache for the extravagant. We layer mozzarella and provolone, then add thin sliced oven roasted organic red and yukon gold potatoes, thick cut applewood bacon, red onions and fresh picked rosemary. We finish it with a white truffle oil drizzle and Parmigiano-Reggiano. NO SAUCE- Roasted garlic & olive oil base."};
            ++id;
            store.items[id] = {id, 5, 3, "Kebler",
                               "Colorado is home to one of the largest Aspen groves in the world, a single sprawling mass connected via a single root system located in Kebler Pass. After seeing those leaves in autumn we knew we had to name a pizza after it. So we take a pie cover it in our house pesto pomodoro sauce, mozzarella and add garlic fennel Italian sausage, green and tri-colored bell peppers."};
            ++id;
            store.items[id] = {id, 5, 4, "The Pueblo",
                               "Pueblo is not only known as a a southern neighbor but also a haven for some of the state's best chili producers. We take a pie and spread a layer of creamy neufchatel cheese and fire roasted poblano peppers then cover in smoked gouda and mozzarella and finish with roasted corn."};
            ++id;
            store.items[id] = {id, 5, 4, "The Greek",
                               "A base of EVOO, roasted garlic and Greek oregano is covered in a layer of mozzarella. We add artichoke hearts, organic spinach, imported Mykonos kalamata olives, imported Sicilian sun dried tomatoes, red onions and finished with fresh feta crumble."};
            ++id;
            store.items[id] = {id, 5, 4, "Molly Brown",
                               "This pizza is just gold! Our house pomodoro covered in mozzarella and aged Tilamook sharp cheddar with applewood smoked bacon, roma tomatoes, roasted garlic and fresh organic spinach."};
            ++id;
            store.items[id] = {id, 5, 5, "Clyde & Chauncey",
                               "The infamous \"Don of Denver\" and his brother ran their criminal enterprises out of a restaurant in North Denver where their mama would hand roll meatballs that were nearly as famous as her sons. So it only seemed right we named this classic after those two meatball mafiosi. We start with our classic pie and add a mozzarella and provolone blend then cover in sliced house made and hand rolled meatballs(70%beef/30%pork) and caramelized onions. We finish the pie with grated Parmigiano-Reggiano and fresh Italian parsley."};
            ++id;
            store.items[id] = {id, 5, 5, "1876",
                               "We layer a pie with our red sauce, mozzarella and provolone and then add tri-colored bell peppers, white button mushrooms, red onions, black olives and imported pepperoni."};
            ++id;
            store.items[id] = {id, 5, 6, "Rankin Kelly",
                               "El Paso County's 1st sheriff was a character legends were built on. Rough and formidable on the outside and surprisingly soft and tender on the inside, Rankin was a man to be admired. We realized we had a pizza just like him! We take our classic pie and layer globs of fresh ricotta impastata then cover with mozzarella and provolone blend and top with sliced house made meatballs, garlic & fennel Italian sausage and organic spinach."};
            ++id;
            store.items[id] = {id, 5, 6, "BLT",
                               "We love the sandwich so much we had to make a pie to pay homage. We start with a roasted garlic & olive oil base then cover in mozzarella and provolone. We layer juicy Organic heirloom tomatoes and thick cut apple-wood bacon on top and finish with Organic arugula and a balsamic reduction drizzle."};
            ++id;
            store.items[id] = {id, 5, 7, "Mt Massive.",
                               "In an attempt to woo the carnivores out there, we layer our classic pie with sliced house made meatballs, imported pepperoni, Italian sausage, spiral cut ham and thick cut applewood bacon."};
        }
    }

    ~fake_shop() {
        application_quits = true;

        // Wait for transactions
        for (auto &t : transaction_threads) t.join();
        for (auto &t : nspv_threads) t.join();
    }

    void display_balance(const inventory &inv, bool show_pending_count = false) {
        std::scoped_lock st_lock(store_mutex, user_mutex);
        if (show_pending_count)
            ImGui::Text("Pending Transactions: %d, Mempool: %d", pending_transaction_count, mempool_transaction_count);

        bool pending = inv.pending_balance != 0;
        ImGui::Text(std::string(std::string("Balance: %.2lf") +
                                std::string(
                                        !pending ? "" : std::string(inv.pending_balance > 0 ? "(+" : "(") + "%.2lf)") +
                                std::string("    nSPV Balance: %.2lf")).c_str(),
                    inv.balance, pending ? inv.pending_balance : inv.nspv_balance, inv.nspv_balance);
    }

    bool user_has_enough_funds(int price) {
        std::scoped_lock lock(user_mutex);
        return user.balance >= price;
    }

    bool wait_until_pending_completion(blockchain::nspv_tx_answer tx) {
        bool success = false;
        bool done = false;
        while (!application_quits && !done) {
            std::this_thread::sleep_for(10s);
            if (!nspv_system_user_.is_transaction_pending(currency, tx.broadcast_answer.value().broadcast,
                                                          tx.send_answer.vout)) {
                done = true;
                // TODO: Set success correctly here
                success = true;
            }
        }
        return success;
    }

    void clear_pending(const int price) {
        user.pending_balance += price;
        store.pending_balance -= price;
        --pending_transaction_count;
    }

    void pay_and_transfer(item &store_item) {
        const int price = store_item.price;

        // Exchange balance
        user.balance -= price;
        store.balance += price;

        // Set pending balance
        ++pending_transaction_count;

        user.pending_balance -= price;
        store.pending_balance += price;

        // Try send asynchronously
        if (!application_quits)
            transaction_threads.emplace_back([this, price, &store_item] {
                std::cout << "Sending " << price << " to " << store.wallet_address << "..." << std::endl;
                auto tx = nspv_system_user_.send(currency, store.wallet_address, price);

                // If payment is successful, check for pending
                if (tx.broadcast_answer.has_value()) {
                    {
                        std::scoped_lock lck(tx_ids_mutex);
                        tx_ids.push_back(tx.broadcast_answer.value().broadcast);
                    }
                    std::cout << "Send complete, Transaction ID: " << tx.broadcast_answer.value().broadcast
                              << std::endl;

                    // Check pending status
                    if (!application_quits)
                        transaction_threads.emplace_back([this, price, tx, &store_item] {
                            bool success = wait_until_pending_completion(tx);
                            clear_pending(price);
                            post_payment_tasks(success, &store_item);
                        });
                }
                    // If payment failed, no need to wait for pending
                else {
                    post_payment_tasks(false, &store_item);
                }
            });
    }

    void pre_payment_tasks(item &store_item) {
        // Lower the stock
        --store_item.quantity;
    }

    void post_payment_tasks(bool success, item *store_item = nullptr) {
        // If payment is successful,
        if (success) {
            // Transfer the item
            transfer_item(*store_item);
        } else {
            // Increase the stock back
            ++store_item->quantity;
        }
    }

    void transfer_item(item &store_item) {
        // Give the item
        // If user already has this item, just increase the quantity
        if (user.items.find(store_item.id) != user.items.end()) {
            ++user.items[store_item.id].quantity;
        }
            // Else, add this item, set quantity as 1
        else {
            user.items[store_item.id] = store_item;
            user.items[store_item.id].quantity = 1;
        }
    }

    void sell(item &store_item) {
        // Prepare the payment
        pre_payment_tasks(store_item);

        // Pay and transfer on success
        pay_and_transfer(store_item);
    }

    void update() noexcept final {
        // Inventory
        {
            ImGui::Begin("Inventory");

            display_balance(user);

            ImGui::Separator();

            // Items
            auto &items = user.items;
            auto &target_list = store.items;
            for (auto it = items.begin(); it != items.end(); ++it) {
                auto &item = it->second;
                ImGui::PushID(item.id);

                if (ImGui::Button(std::string(std::to_string(item.quantity) + "x " + item.name).c_str())) {}

                ImGui::PopID();
            }

            ImGui::End();
        }


        // Store
        {
            ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Store")) {
                display_balance(store, true);

                auto &items = store.items;

                // Left
                static int curr_item_id = items.begin()->second.id;
                static int selected = 0;

                ImGui::BeginChild("left pane", ImVec2(150, 0), true);
                int i = 0;
                for (auto it = items.begin(); it != items.end(); ++it, ++i) {
                    auto &item = it->second;
                    if (ImGui::Selectable(item.name.c_str(), selected == i)) {
                        selected = i;
                        curr_item_id = item.id;
                    }
                }
                ImGui::EndChild();
                ImGui::SameLine();

                // Right
                auto &curr_item = items[curr_item_id];
                ImGui::BeginGroup();
                ImGui::BeginChild("item view",
                                  ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
                ImGui::TextWrapped("%s  x%d", curr_item.name.c_str(), curr_item.quantity);
                ImGui::Separator();
                if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
                    if (ImGui::BeginTabItem("Description")) {
                        ImGui::TextWrapped("%s", curr_item.description.c_str());
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Details")) {
                        ImGui::Text("ID: %d", curr_item.id);
                        ImGui::Text("Quantity: %d", curr_item.quantity);
                        ImGui::Text("Unit Price: %.2lf %s", curr_item.price, currency.c_str());
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndChild();

                bool has_stock = curr_item.quantity > 0;
                bool has_funds = user_has_enough_funds(curr_item.price);
                if (ImGui::Button(std::string(
                        !has_funds ? std::string(
                                "Not enough funds (" + double_to_str(curr_item.price) + " " + currency + ")") :
                        !has_stock ? "Out of stock" :
                        ("Buy 1 for " + double_to_str(curr_item.price) + " " + currency)).c_str())) {
                    if (has_stock && has_funds) {
                        sell(curr_item);
                    }
                }
                ImGui::SameLine();
                ImGui::EndGroup();
            }
            ImGui::End();
        }

        // Transactions
        {
            ImGui::Begin("Transactions");
            {
                std::scoped_lock lck(tx_ids_mutex);
                ImGui::Text("%d Transactions", tx_ids.size());
            }

            ImGui::Separator();

            // Items
            {
                std::scoped_lock lck(tx_ids_mutex);
                for (auto &tx_id : tx_ids) {
                    if (ImGui::Button(std::string(tx_id).c_str())) {
                        core::open_url_browser("http://" + currency_lc + "." + explorer_url + "/tx/" + tx_id);
                    }
                }
            }

            ImGui::End();
        }
    }

    // State
    std::atomic<bool> application_quits{false};

    // Inventories
    std::mutex store_mutex;
    std::mutex user_mutex;
    inventory store{0};
    inventory user{0};

    // Threads
    int pending_transaction_count{0};
    int mempool_transaction_count{0};
    std::vector<std::thread> transaction_threads;
    std::vector<std::thread> nspv_threads;

    std::mutex tx_ids_mutex;
    std::vector<std::string> tx_ids;

    // nSPV
    blockchain::nspv &nspv_system_user_;
    blockchain::nspv nspv_system_store_{entity_registry_};
};

REFL_AUTO(type(fake_shop))

class my_world : public world::app {
public:
    my_world() noexcept {
        auto &nspv_system = this->system_manager_.create_system<blockchain::nspv>();
        nspv_system.spawn_nspv_instance(currency, true);

        auto &graphic_system = this->system_manager_.create_system<sfml::graphic_system>();
        this->entity_registry_.set<sfml::resources_system>(this->entity_registry_);
        this->system_manager_.create_system<sfml::input_system>(graphic_system.get_window());
        system_manager_.create_system<fake_shop>(nspv_system);
        system_manager_.prioritize_system<fake_shop, sfml::graphic_system>();
    }
};


int main() {
    assert(std::getenv("SECRET_WIF_WALLET") != nullptr);
    assert(std::getenv("SHOP_WIF_WALLET") != nullptr);

    my_world world;
    return world.run();
}