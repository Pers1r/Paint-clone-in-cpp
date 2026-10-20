#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include "imgui/ImGuiFileDialog.h"

#include <iostream>
#include <cmath>
#include <filesystem>
#include <string>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(
        sf::VideoMode({ 1800, 900 }),
        "ImGui + SFML",
        sf::Style::Titlebar | sf::Style::Close
    );
    sf::Clock deltaClock;

    window.setFramerateLimit(60);
    std::ignore = ImGui::SFML::Init(window);

    // Tools
    enum class Tool {
        None,
        Broom,
        Circle,
        Window,
        Square
    };

    Tool current_tool = Tool::None;

    std::string current_file_path;

    // States
    bool show_tool_options = false;
    float border_thickness = 2.0f;
    bool line_gradient = false;
    bool show_exit_popup = false;
    bool show_about_popup = false;

    // Canvas
    sf::RenderTexture canvas({ 1800, 900 });
    canvas.clear(sf::Color::Black);
    canvas.display();

    // Mouse
    bool is_drawing = false;
    sf::Vector2f start_pos(0.f, 0.f);
    sf::Vector2f current_mouse_pos(0.f, 0.f);

    // Colors
    ImVec4 border_color({ 1.0f, 1.0f, 1.0f, 1.0f });
    ImVec4 fill_color({ 0.0f, 1.0f, 0.0f, 1.0f });

    auto toSfColor = [](const ImVec4& c) {
        return sf::Color(
            static_cast<unsigned char>(c.x * 255),
            static_cast<unsigned char>(c.y * 255),
            static_cast<unsigned char>(c.z * 255),
            static_cast<unsigned char>(c.w * 255)
        );
    };

    // Images loading
    sf::Texture broom_button("broom.png");
    sf::Sprite broom_sprite(broom_button);
    sf::Texture circle_button("circle.png");
    sf::Sprite circle_sprite(circle_button);
    sf::Texture square_button("square.png");
    sf::Sprite square_sprite(square_button);
    sf::Texture window_button("window.png");
    sf::Sprite window_sprite(window_button);

    while (window.isOpen()) {

        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>()) {
                show_exit_popup = true;
            }

            if (!ImGui::GetIO().WantCaptureMouse) {
                if (const auto* mousePress = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mousePress->button == sf::Mouse::Button::Left && current_tool != Tool::None) {
                        is_drawing = true;
                        start_pos = sf::Vector2f(static_cast<float>(mousePress->position.x), static_cast<float>(mousePress->position.y));
                        current_mouse_pos = start_pos;
                    }
                }
                if (const auto* mouseMove = event->getIf<sf::Event::MouseMoved>()) {
                    current_mouse_pos = sf::Vector2f(static_cast<float>(mouseMove->position.x), static_cast<float>(mouseMove->position.y));
                }
                if (const auto* mouseRelease = event->getIf<sf::Event::MouseButtonReleased>()) {
                    if (mouseRelease->button == sf::Mouse::Button::Left && is_drawing) {
                        is_drawing = false;
                        sf::Vector2f end_pos(static_cast<float>(mouseRelease->position.x), static_cast<float>(mouseRelease->position.y));

                        auto sfBorder = toSfColor(border_color);
                        auto sfFill = toSfColor(fill_color);

                        if (current_tool == Tool::Broom) {
                            if (line_gradient) {
                                sf::Vertex line[] = { {start_pos, sfBorder}, {end_pos, sfFill} };
                                canvas.draw(line, 2, sf::PrimitiveType::Lines);
                            } else {
                                sf::Vertex line[] = { {start_pos, sfBorder}, {end_pos, sfBorder} };
                                canvas.draw(line, 2, sf::PrimitiveType::Lines);
                            }
                        }
                        else if (current_tool == Tool::Square) {
                            sf::RectangleShape rect;
                            rect.setPosition({std::min(start_pos.x, end_pos.x), std::min(start_pos.y, end_pos.y)});
                            rect.setSize({std::abs(end_pos.x - start_pos.x), std::abs(end_pos.y - start_pos.y)});
                            rect.setFillColor(sf::Color::Transparent);
                            rect.setOutlineColor(sfBorder);
                            rect.setOutlineThickness(border_thickness);
                            canvas.draw(rect);
                        }
                        else if (current_tool == Tool::Window) {
                            sf::RectangleShape rect;
                            rect.setPosition({std::min(start_pos.x, end_pos.x), std::min(start_pos.y, end_pos.y)});
                            rect.setSize({std::abs(end_pos.x - start_pos.x), std::abs(end_pos.y - start_pos.y)});
                            rect.setFillColor(sfFill);
                            rect.setOutlineColor(sfBorder);
                            rect.setOutlineThickness(border_thickness);
                            canvas.draw(rect);
                        }
                        else if (current_tool == Tool::Circle) {
                            float radius = std::hypot(end_pos.x - start_pos.x, end_pos.y - start_pos.y);
                            sf::CircleShape circle(radius);
                            circle.setOrigin({radius, radius});
                            circle.setPosition(start_pos);
                            circle.setFillColor(sf::Color::Transparent);
                            circle.setOutlineColor(sfBorder);
                            circle.setOutlineThickness(border_thickness);
                            canvas.draw(circle);
                        }
                        canvas.display();
                    }
                }
            }
        }


        ImGui::SFML::Update(window, deltaClock.restart());

        // MENU
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New")) {
                    canvas.clear(sf::Color::Black);
                    canvas.display();

                    current_file_path.clear();
                }
                if (ImGui::MenuItem("Open")) {
                    IGFD::FileDialogConfig config;
                    config.path = "..";

                    ImGuiFileDialog::Instance()->OpenDialog(
                        "OpenImageDialog",
                        "Open Image",
                        "Image files (*.png *.jpg *.jpeg){.png,.jpg,.jpeg}",
                        config
                    );
                }
                if (ImGui::MenuItem("Save as")) {
                    IGFD::FileDialogConfig config;
                    config.path = "..";
                    config.fileName = "drawing.png";
                    config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

                    ImGuiFileDialog::Instance()->OpenDialog(
                        "SaveImageDialog",
                        "Save Drawing As",
                        "PNG image (*.png){.png}",
                        config
                    );
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    show_exit_popup = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Show Tool Options", nullptr, &show_tool_options);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("About")) {
                    show_about_popup = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        auto* fileDialog = ImGuiFileDialog::Instance();

        // OPEN IMAGE
        if (fileDialog->Display("OpenImageDialog")) {
            if (fileDialog->IsOk()) {
                const std::string path = fileDialog->GetFilePathName();

                try {
                    sf::Texture loadedTexture(path);
                    sf::Sprite loadedSprite(loadedTexture);

                    canvas.clear(sf::Color::Black);
                    canvas.draw(loadedSprite);
                    canvas.display();

                    current_file_path = path;

                    std::cout << "Opened: " << path << '\n';
                }
                catch (const std::exception& e) {
                    std::cerr << "Could not open image: "
                              << e.what() << '\n';
                }
            }

            fileDialog->Close();
        }

        // SAVE AS
        if (fileDialog->Display("SaveImageDialog")) {
            if (fileDialog->IsOk()) {
                std::string path = fileDialog->GetFilePathName();

                if (std::filesystem::path(path).extension().empty()) {
                    path += ".png";
                }

                sf::Image image = canvas.getTexture().copyToImage();

                if (image.saveToFile(path)) {
                    current_file_path = path;
                    std::cout << "Saved: " << path << '\n';
                }
                else {
                    std::cerr << "Could not save image: "
                              << path << '\n';
                }
            }

            fileDialog->Close();
        }

        // POPUPS
        if (show_exit_popup) {
            ImGui::OpenPopup("Exit Confirmation");
        }
        if (ImGui::BeginPopupModal("Exit Confirmation", &show_exit_popup, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Are you sure you want to exit the program?");
            if (ImGui::Button("Yes", { 120, 0 })) {
                window.close();
            }
            ImGui::SameLine();
            if (ImGui::Button("No", { 120, 0 })) {
                show_exit_popup = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (show_about_popup) {
            ImGui::OpenPopup("About");
        }
        if (ImGui::BeginPopupModal("About", &show_about_popup, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("IUiBG Lab 02\nAutor: Markiian Novytskyi");
            if (ImGui::Button("OK", { 120, 0 })) {
                show_about_popup = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        auto getButtonTint = [&](Tool tool) {
            return (current_tool == tool) ? sf::Color(0, 255, 0) : sf::Color::White;
        };

        // TOOLS
        ImGui::Begin("Tools");
            if (ImGui::ImageButton("broom", broom_button, { 50,50 }, getButtonTint(Tool::Broom))) {
                current_tool = Tool::Broom;
            }
            ImGui::SameLine();

            if (ImGui::ImageButton("circle", circle_button, { 50,50 }, getButtonTint(Tool::Circle))) {
                current_tool = Tool::Circle;
            }

            if (ImGui::ImageButton("window", window_button, { 50,50 }, getButtonTint(Tool::Window))) {
                current_tool = Tool::Window;
            }
            ImGui::SameLine();

            if (ImGui::ImageButton("square", square_button, { 50,50 }, getButtonTint(Tool::Square))) {
                current_tool = Tool::Square;
            }
            ImGui::Separator();

            ImGui::Text("Border color");
            ImGui::ColorEdit3("Border color", (float*)&border_color, ImGuiColorEditFlags_NoLabel);
            ImGui::Text("Fill color");
            ImGui::ColorEdit3("Fill color", (float*)&fill_color, ImGuiColorEditFlags_NoLabel);

            if (show_tool_options && current_tool != Tool::None) {
                ImGui::Separator();
                ImGui::Text("Tool Options");
                if (current_tool == Tool::Broom) {
                    ImGui::Checkbox("Gradient Line", &line_gradient);
                } else {
                    ImGui::SliderFloat("Thickness", &border_thickness, 1.0f, 20.0f);
                }
            }
        ImGui::End();

        window.clear(sf::Color::Black);

        sf::Sprite canvas_sprite(canvas.getTexture());
        window.draw(canvas_sprite);

        // PREVIEW
        if (is_drawing) {
            auto sfBorder = toSfColor(border_color);
            auto sfFill = toSfColor(fill_color);

            if (current_tool == Tool::Broom) {
                if (line_gradient) {
                    sf::Vertex line[] = { {start_pos, sfBorder}, {current_mouse_pos, sfFill} };
                    window.draw(line, 2, sf::PrimitiveType::Lines);
                } else {
                    sf::Vertex line[] = { {start_pos, sfBorder}, {current_mouse_pos, sfBorder} };
                    window.draw(line, 2, sf::PrimitiveType::Lines);
                }
            }
            else if (current_tool == Tool::Square) {
                sf::RectangleShape rect;
                rect.setPosition({std::min(start_pos.x, current_mouse_pos.x), std::min(start_pos.y, current_mouse_pos.y)});
                rect.setSize({std::abs(current_mouse_pos.x - start_pos.x), std::abs(current_mouse_pos.y - start_pos.y)});
                rect.setFillColor(sf::Color::Transparent);
                rect.setOutlineColor(sfBorder);
                rect.setOutlineThickness(border_thickness);
                window.draw(rect);
            }
            else if (current_tool == Tool::Window) {
                sf::RectangleShape rect;
                rect.setPosition({std::min(start_pos.x, current_mouse_pos.x), std::min(start_pos.y, current_mouse_pos.y)});
                rect.setSize({std::abs(current_mouse_pos.x - start_pos.x), std::abs(current_mouse_pos.y - start_pos.y)});
                rect.setFillColor(sfFill);
                rect.setOutlineColor(sfBorder);
                rect.setOutlineThickness(border_thickness);
                window.draw(rect);
            }
            else if (current_tool == Tool::Circle) {
                float radius = std::hypot(current_mouse_pos.x - start_pos.x, current_mouse_pos.y - start_pos.y);
                sf::CircleShape circle(radius);
                circle.setOrigin({radius, radius});
                circle.setPosition(start_pos);
                circle.setFillColor(sf::Color::Transparent);
                circle.setOutlineColor(sfBorder);
                circle.setOutlineThickness(border_thickness);
                window.draw(circle);
            }
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}