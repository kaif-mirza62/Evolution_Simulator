#include "raylib.h"
#include "simulation.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

enum class Screen{
    Menu,
    Simulation,
    Results
};

const Color BACKGROUND = {238, 242, 238, 255};
const Color PANEL = {255, 255, 255, 255};
const Color PANEL_DARK = {43, 48, 54, 255};
const Color TEXT = {35, 39, 43, 255};
const Color MUTED = {105, 112, 120, 255};
const Color ACCENT = {69, 136, 82, 255};
const Color ACCENT_LIGHT = {223, 240, 225, 255};
const Color PREY_COLOR = BLACK;
const Color PREDATOR_COLOR = RED;
const Color FOOD_COLOR = GREEN;

bool drawButton(
    Rectangle rectangle,
    const char* text,
    bool enabled = true
)
{
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rectangle);

    Color background = enabled ? (hover ? ACCENT : PANEL_DARK) : Color{170, 175, 180, 255};
    
    DrawRectangleRounded(
        rectangle,
        0.22f,
        12,
        background
    );

    int fontSize = (int)(rectangle.height * 0.42f);
    int textWidth = MeasureText(text, fontSize);

    DrawText(
        text,
        (int)(rectangle.x + (rectangle.width - textWidth) / 2),
        (int)(rectangle.y + rectangle.height / 2 - fontSize / 2),
        fontSize,
        RAYWHITE
    );

    return enabled && hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool drawValueControl(
    Rectangle rectangle,
    const char* label,
    int value,
    int minimum,
    int maximum,
    int step,
    int& outputValue
)
{
    DrawText(
        label,
        (int)(rectangle.x),
        (int)(rectangle.y + 8),
        20,
        TEXT
    );

    char valueText[32];
    std::snprintf(valueText, sizeof(valueText), "%d", value);

    int valueWidth = MeasureText(valueText, 22);

    DrawText(
        valueText,
        (int)(rectangle.x + rectangle.width / 2 - valueWidth / 2),
        (int)(rectangle.y + 7),
        22,
        ACCENT
    );

    Rectangle minusButton{
        rectangle.x,
        rectangle.y + 36,
        rectangle.width * 0.46f,
        38
    };

    Rectangle plusButton{
        rectangle.x + rectangle.width * 0.54f,
        rectangle.y + 36,
        rectangle.width * 0.46f,
        38
    };

    bool changed = false;

    if (drawButton(minusButton, "-"))
    {
        outputValue = std::max(minimum, value - step);
        changed = true;
    }

    if (drawButton(plusButton, "+"))
    {
        outputValue = std::min(maximum, value + step);
        changed = true;
    }

    return changed;
}

void drawStatCard(
    float x,
    float y,
    float width,
    float height,
    const char* label,
    const char* value,
    Color accent
)
{
    DrawRectangleRounded(
        Rectangle{x, y, width, height},
        0.18f,
        10,
        PANEL
    );

    DrawCircle(
        (int)(x + 25),
        (int)(y + 25),
        7,
        accent
    );

    DrawText(
        label,
        (int)(x + 42),
        (int)(y + 14),
        17,
        MUTED
    );

    DrawText(
        value,
        (int)(x + 20),
        (int)(y + 40),
        26,
        TEXT
    );
}

void drawTraitRow(
    float x,
    float y,
    float width,
    const char* name,
    float initialValue,
    float finalValue
)
{
    DrawText(
        name,
        (int)(x),
        (int)(y),
        18,
        TEXT
    );

    char text[64];

    std::snprintf(
        text,
        sizeof(text),
        "%.1f  ->  %.1f",
        initialValue,
        finalValue
    );

    DrawText(
        text,
        (int)(x + width - 50),
        (int)(y),
        18,
        MUTED
    );

    float change = 0.0f;

    if (std::fabs(initialValue) > 0.001f)
    {
        change = ((finalValue - initialValue) / initialValue) * 100.0f;
    }

    char changeText[32];

    std::snprintf(
        changeText,
        sizeof(changeText),
        "%+.1f%%",
        change
    );

    DrawText(
        changeText,
        (int)(x + width - 45),
        (int)(y),
        18,
        change >= 0.0f ? ACCENT : RED
    );
}

void drawMenu(
    SimulationConfig& config,
    Screen& currentScreen,
    Simulation& simulation,
    bool& runningExperiment
)
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    ClearBackground(BACKGROUND);

    DrawText(
        "EVOLUTION LAB",
        70,
        55,
        48,
        TEXT
    );

    DrawText(
        "Build an ecosystem. Watch it evolve.",
        74,
        110,
        22,
        MUTED
    );

    float panelWidth = std::min(520.0f, screenWidth * 0.72f);
    float panelX = (screenWidth - panelWidth) / 2.0f;
    float panelY = 160.0f;

    DrawRectangleRounded(
        Rectangle{panelX, panelY, panelWidth, 370.0f},
        0.05f,
        12,
        PANEL
    );

    DrawText(
        "EXPERIMENT SETUP",
        (int)(panelX + 30),
        (int)(panelY + 25),
        26,
        TEXT
    );

    float left = panelX + 30.0f;
    float controlWidth = (panelWidth - 70.0f) / 2.0f;

    drawValueControl(
        Rectangle{left, panelY + 75, controlWidth, 78},
        "Prey",
        config.initialPrey,
        5,
        200,
        5,
        config.initialPrey
    );

    drawValueControl(
        Rectangle{left + controlWidth + 10, panelY + 75, controlWidth, 78},
        "Predators",
        config.initialPredators,
        1,
        80,
        1,
        config.initialPredators
    );

    float foodControlX = panelX + (panelWidth - controlWidth) / 2.0f;

    drawValueControl(
        Rectangle{foodControlX, panelY + 180, controlWidth, 78},
        "Food",
        config.initialFood,
        20,
        500,
        20,
        config.initialFood
    );

    Rectangle startButton{
        panelX + 30,
        panelY + 290,
        panelWidth - 60,
        55
    };

    if (drawButton(startButton, "START EXPERIMENT"))
    {
        simulation.start(config);
        runningExperiment = true;
        currentScreen = Screen::Simulation;
    }

    DrawText(
        "F  Fullscreen     1-5  Simulation speed     SPACE  Pause",
        70,
        screenHeight - 45,
        16,
        MUTED
    );
}

void drawLearningSummary(
    float x,
    float y,
    float width,
    Simulation& simulation
)
{
    DrawRectangleRounded(
        Rectangle{x, y, width, 72.0f},
        0.12f,
        10,
        Color{247, 249, 247, 255}
    );

    const float colWidth = width / 2.0f;
    const int labelSize = 12;
    const int valueSize = 14;

    DrawText("MEMORY", (int)x + 12, (int)y + 9, labelSize, MUTED);
    DrawText("TRAINING", (int)(x + colWidth + 8), (int)y + 9, labelSize, MUTED);

    char text[64];

    std::snprintf(
        text,
        sizeof(text),
        "P:%zu  D:%zu",
        simulation.getPreyMemorySize(),
        simulation.getPredatorMemorySize()
    );
    DrawText(text, (int)x + 12, (int)y + 27, valueSize, TEXT);

    std::snprintf(
        text,
        sizeof(text),
        "P:%zu  D:%zu",
        simulation.getPreyTrainingSteps(),
        simulation.getPredatorTrainingSteps()
    );
    DrawText(text, (int)(x + colWidth + 8), (int)y + 27, valueSize, TEXT);

    DrawText("EPSILON", (int)x + 12, (int)y + 48, labelSize, MUTED);
    DrawText("LOSS", (int)(x + colWidth + 8), (int)y + 48, labelSize, MUTED);

    std::snprintf(
        text,
        sizeof(text),
        "P:%.2f  D:%.2f",
        simulation.getPreyEpsilon(),
        simulation.getPredatorEpsilon()
    );
    DrawText(text, (int)x + 12, (int)y + 61, 11, TEXT);

    std::snprintf(
        text,
        sizeof(text),
        "P:%.3f  D:%.3f",
        simulation.getPreyLoss(),
        simulation.getPredatorLoss()
    );
    DrawText(text, (int)(x + colWidth + 8), (int)y + 61, 11, TEXT);
}

void drawSelectedCreatureCard(
    float x,
    float y,
    float width,
    float height,
    const Creature* selected
)
{
    DrawRectangleRounded(
        Rectangle{x + 2, y + 3, width, height},
        0.12f,
        10,
        Color{225, 229, 225, 120}
    );

    DrawRectangleRounded(
        Rectangle{x, y, width, height},
        0.12f,
        10,
        PANEL
    );

    if (selected == nullptr)
    {
        DrawText("CREATURE DETAILS", (int)x + 14, (int)y + 12, 15, MUTED);
        DrawText(
            "Click a prey or predator",
            (int)x + 14,
            (int)y + 35,
            16,
            TEXT
        );
        DrawText(
            "to inspect its current state.",
            (int)x + 14,
            (int)y + 56,
            14,
            MUTED
        );
        return;
    }

    const bool isPrey = selected->type == CreatureType::Prey;
    const Color typeColor = isPrey ? TEXT : PREDATOR_COLOR;
    const char* typeName = isPrey ? "PREY" : "PREDATOR";

    DrawCircle((int)x + 17, (int)y + 19, 6, typeColor);
    DrawText(typeName, (int)x + 30, (int)y + 9, 17, typeColor);

    char line[128];

    std::snprintf(line, sizeof(line), "ID %d", selected->id);
    int idWidth = MeasureText(line, 13);
    DrawText(
        line,
        (int)(x + width - idWidth - 14),
        (int)y + 11,
        13,
        MUTED
    );

    const char* actionName = "Wander";

    if (selected->brain.currentAction == Action::FindFood)
        actionName = "Finding food";
    else if (selected->brain.currentAction == Action::Escape)
        actionName = "Escaping";
    else if (selected->brain.currentAction == Action::Hunt)
        actionName = "Hunting";
    else if (selected->brain.currentAction == Action::Rest)
        actionName = "Resting";

    std::snprintf(line, sizeof(line), "Brain  %s", actionName);
    DrawText(line, (int)x + 14, (int)y + 37, 14, TEXT);

    // Energy bar: normalize to a readable 0-100% display without
    // assuming anything about the simulation's internal max-energy value.
    const float normalizedEnergy = std::max(
        0.0f,
        std::min(100.0f, selected->energy)
    ) / 100.0f;

    DrawText("ENERGY", (int)x + 14, (int)y + 61, 11, MUTED);

    Rectangle energyBar{
        x + 14,
        y + 78,
        width - 28,
        9
    };

    DrawRectangleRounded(
        energyBar,
        0.5f,
        6,
        Color{225, 229, 225, 255}
    );

    Rectangle energyFill = energyBar;
    energyFill.width *= normalizedEnergy;

    if (energyFill.width > 0.0f)
    {
        DrawRectangleRounded(
            energyFill,
            0.5f,
            6,
            ACCENT
        );
    }

    std::snprintf(
        line,
        sizeof(line),
        "%.0f",
        selected->energy
    );

    DrawText(
        line,
        (int)(x + width - MeasureText(line, 11) - 14),
        (int)y + 58,
        11,
        TEXT
    );

    if (height >= 128.0f)
    {
        std::snprintf(
            line,
            sizeof(line),
            "Speed %.1f    Vision %.0f",
            selected->speed,
            selected->vision
        );

        DrawText(
            line,
            (int)x + 14,
            (int)y + 101,
            14,
            MUTED
        );
    }
}

void drawSimulation(
    Simulation& simulation,
    Screen& currentScreen,
    int& selectedCreatureId,
    int& simulationSpeed,
    bool& paused
)
{
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    // Keep enough width for readable controls even on smaller windows,
    // while allowing the world to use most of the screen on wide displays.
    float panelWidth = std::max(300.0f, screenWidth * 0.30f);
    panelWidth = std::min(panelWidth, 380.0f);
    panelWidth = std::min(panelWidth, screenWidth * 0.46f);

    // If the window gets extremely small, keep the sidebar usable without
    // making its width larger than the window.
    panelWidth = std::max(240.0f, std::min(panelWidth, (float)screenWidth - 220.0f));

    const float worldWidth = std::max(1.0f, (float)screenWidth - panelWidth);
    const float worldHeight = (float)screenHeight;
    const float panelX = worldWidth;
    const float contentX = panelX + 18.0f;
    const float contentWidth = panelWidth - 36.0f;

    simulation.setWorldSize(worldWidth, worldHeight);
    simulation.draw(worldWidth, worldHeight);

    // Sidebar
    DrawRectangle(
        (int)panelX,
        0,
        (int)panelWidth,
        screenHeight,
        Color{248, 250, 248, 255}
    );

    DrawLine(
        (int)panelX,
        0,
        (int)panelX,
        screenHeight,
        Color{220, 225, 220, 255}
    );

    DrawText(
        "LIVE ECOSYSTEM",
        (int)contentX,
        22,
        25,
        TEXT
    );

    char generationText[64];

    std::snprintf(
        generationText,
        sizeof(generationText),
        "GENERATION %d / %d",
        simulation.getGeneration(),
        simulation.getGenerationCount()
    );

    DrawText(
        generationText,
        (int)contentX,
        55,
        14,
        MUTED
    );

    Rectangle progressBar{
        contentX,
        80,
        contentWidth,
        10
    };

    DrawRectangleRounded(
        progressBar,
        0.5f,
        8,
        Color{220, 225, 220, 255}
    );

    progressBar.width *= std::max(
        0.0f,
        std::min(1.0f, simulation.getGenerationProgress())
    );

    DrawRectangleRounded(
        progressBar,
        0.5f,
        8,
        ACCENT
    );

    char preyText[32];
    char predatorText[32];
    char foodText[32];

    std::snprintf(preyText, sizeof(preyText), "%d", simulation.getPreyCount());
    std::snprintf(predatorText, sizeof(predatorText), "%d", simulation.getPredatorCount());
    std::snprintf(foodText, sizeof(foodText), "%d", simulation.getFoodCount());

    const float cardHeight = 62.0f;
    const float cardGap = 9.0f;

    drawStatCard(
        contentX,
        108,
        contentWidth,
        cardHeight,
        "PREY",
        preyText,
        PREY_COLOR
    );

    drawStatCard(
        contentX,
        108 + cardHeight + cardGap,
        contentWidth,
        cardHeight,
        "PREDATORS",
        predatorText,
        PREDATOR_COLOR
    );

    drawStatCard(
        contentX,
        108 + (cardHeight + cardGap) * 2.0f,
        contentWidth,
        cardHeight,
        "FOOD",
        foodText,
        FOOD_COLOR
    );

    const bool shortLayout = screenHeight < 680;
    const bool ultraCompactLayout = screenHeight < 560;

    float speedTitleY;

    if (!shortLayout)
    {
        const float learningY = 324.0f;

        DrawText(
            "LEARNING",
            (int)contentX,
            (int)learningY,
            14,
            MUTED
        );

        drawLearningSummary(
            contentX,
            learningY + 20.0f,
            contentWidth,
            simulation
        );

        speedTitleY = learningY + 101.0f;
    }
    else
    {
        // On short windows, collapse learning into one compact row so the
        // creature inspector always has room at the bottom of the panel.
        char compactLearning[128];

        std::snprintf(
            compactLearning,
            sizeof(compactLearning),
            "Learning  M:%zu/%zu  T:%zu/%zu  E:%.2f/%.2f",
            simulation.getPreyMemorySize(),
            simulation.getPredatorMemorySize(),
            simulation.getPreyTrainingSteps(),
            simulation.getPredatorTrainingSteps(),
            simulation.getPreyEpsilon(),
            simulation.getPredatorEpsilon()
        );

        DrawText(
            compactLearning,
            (int)contentX,
            330,
            11,
            MUTED
        );

        speedTitleY = ultraCompactLayout ? 352.0f : 350.0f;
    }

    DrawText(
        "SIMULATION SPEED",
        (int)contentX,
        (int)speedTitleY,
        14,
        MUTED
    );

    const float speedButtonY = speedTitleY + 23.0f;
    const float speedGap = 5.0f;
    const float speedButtonWidth =
        (contentWidth - speedGap * 4.0f) / 5.0f;

    for (int i = 1; i <= 5; i++)
    {
        Rectangle speedButton{
            contentX + (i - 1) * (speedButtonWidth + speedGap),
            speedButtonY,
            speedButtonWidth,
            34.0f
        };

        const bool active = simulationSpeed == i;
        const bool hover = CheckCollisionPointRec(GetMousePosition(), speedButton);

        DrawRectangleRounded(
            speedButton,
            0.18f,
            8,
            active ? ACCENT : (hover ? Color{61, 68, 75, 255} : PANEL_DARK)
        );

        char speedLabel[8];
        std::snprintf(speedLabel, sizeof(speedLabel), "%dx", i);

        const int textWidth = MeasureText(speedLabel, 14);

        DrawText(
            speedLabel,
            (int)(speedButton.x + (speedButton.width - textWidth) / 2.0f),
            (int)(speedButton.y + 9),
            14,
            RAYWHITE
        );

        if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            simulationSpeed = i;
        }
    }

    Rectangle pauseButton{
        contentX,
        speedButtonY + 44.0f,
        contentWidth,
        40.0f
    };

    const char* pauseText = paused ? "RESUME" : "PAUSE";

    if (drawButton(pauseButton, pauseText))
    {
        paused = !paused;
    }

    // Selection is intentionally handled after drawing the controls so a click
    // on the sidebar can never accidentally select a creature in the world.
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        const Vector2 mouse = GetMousePosition();

        if (mouse.x < worldWidth)
        {
            float bestDistance = 22.0f;
            int bestId = -1;

            for (const Creature& creature : simulation.getCreatures())
            {
                const float dx = mouse.x - creature.x;
                const float dy = mouse.y - creature.y;
                const float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    bestId = creature.id;
                }
            }

            if (bestId != -1)
            {
                selectedCreatureId = bestId;
            }
        }
    }

    const Creature* selected = simulation.findCreature(selectedCreatureId);

    // If a selected creature dies between frames, remove the stale selection
    // instead of leaving the UI pointing at invalid data.
    if (selected == nullptr)
    {
        selectedCreatureId = -1;
    }

    // Selection ring stays in the world, while the detailed state lives in a
    // responsive card inside the sidebar.
    if (selected != nullptr)
    {
        DrawCircleLines(
            (int)selected->x,
            (int)selected->y,
            13.0f,
            ACCENT
        );

        DrawCircleLines(
            (int)selected->x,
            (int)selected->y,
            16.0f,
            Color{69, 136, 82, 90}
        );
    }

    // Reserve the remaining sidebar space for creature details. Unlike the
    // old fixed y=618 layout, this is calculated from the actual window height.
    const float detailsBottomMargin = 12.0f;
    const float rawDetailsY = pauseButton.y + pauseButton.height + 12.0f;
    const float availableDetailsHeight =
        screenHeight - rawDetailsY - detailsBottomMargin;

    if (availableDetailsHeight >= 76.0f)
    {
        const float detailsHeight =
            std::min(170.0f, availableDetailsHeight);

        drawSelectedCreatureCard(
            contentX,
            rawDetailsY,
            contentWidth,
            detailsHeight,
            selected
        );
    }
    else if (screenHeight >= 480)
    {
        // Very short windows get a single compact status strip instead of a
        // panel whose text would extend beyond the bottom edge.
        const float stripHeight = 38.0f;
        const float stripY = screenHeight - stripHeight - 8.0f;

        DrawRectangleRounded(
            Rectangle{contentX, stripY, contentWidth, stripHeight},
            0.12f,
            8,
            PANEL
        );

        char compactSelection[96];

        if (selected != nullptr)
        {
            const char* typeName =
                selected->type == CreatureType::Prey
                    ? "PREY"
                    : "PREDATOR";

            std::snprintf(
                compactSelection,
                sizeof(compactSelection),
                "%s #%d  |  Energy %.0f",
                typeName,
                selected->id,
                selected->energy
            );
        }
        else
        {
            std::snprintf(
                compactSelection,
                sizeof(compactSelection),
                "Click a creature to inspect it"
            );
        }

        DrawText(
            compactSelection,
            (int)contentX + 12,
            (int)stripY + 11,
            13,
            TEXT
        );
    }

    if (IsKeyPressed(KEY_SPACE))
    {
        paused = !paused;
    }

    if (IsKeyPressed(KEY_ONE)) simulationSpeed = 1;
    if (IsKeyPressed(KEY_TWO)) simulationSpeed = 2;
    if (IsKeyPressed(KEY_THREE)) simulationSpeed = 3;
    if (IsKeyPressed(KEY_FOUR)) simulationSpeed = 4;
    if (IsKeyPressed(KEY_FIVE)) simulationSpeed = 5;

    if (simulation.isFinished())
    {
        currentScreen = Screen::Results;
    }
}

void drawResults(
    Simulation& simulation,
    Screen& currentScreen,
    bool& paused
)
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    const SimulationStats& stats = simulation.getStats();

    ClearBackground(BACKGROUND);

    DrawText(
        "EXPERIMENT REPORT",
        60,
        30,
        38,
        TEXT
    );

    const char* outcome = "Balanced ecosystem";
    Color outcomeColor = ACCENT;

    if (stats.finalPrey == 0 && stats.finalPredators == 0)
    {
        outcome = "Ecosystem collapsed";
        outcomeColor = RED;
    }
    else if (stats.finalPrey == 0)
    {
        outcome = "Prey went extinct";
        outcomeColor = RED;
    }
    else if (stats.finalPredators == 0)
    {
        outcome = "Predators went extinct";
        outcomeColor = {210, 130, 40, 255};
    }

    DrawRectangleRounded(
        Rectangle{60, 82, screenWidth - 120.0f, 68},
        0.06f,
        10,
        PANEL
    );

    DrawText(
        outcome,
        85,
        96,
        26,
        outcomeColor
    );

    char generationText[64];
    std::snprintf(
        generationText,
        sizeof(generationText),
        "Generations completed: %d",
        stats.completedGenerations
    );

    DrawText(
        generationText,
        85,
        128,
        15,
        MUTED
    );

    float cardWidth = (screenWidth - 150.0f) / 3.0f;

    char text[32];

    std::snprintf(text, sizeof(text), "%d -> %d", stats.initialPrey, stats.finalPrey);
    drawStatCard(60, 164, cardWidth, 72, "Prey", text, BLACK);

    std::snprintf(text, sizeof(text), "%d -> %d", stats.initialPredators, stats.finalPredators);
    drawStatCard(75 + cardWidth, 164, cardWidth, 72, "Predators", text, RED);

    std::snprintf(text, sizeof(text), "%d", stats.successfulHunts);
    drawStatCard(90 + cardWidth * 2, 164, cardWidth, 72, "Successful Hunts", text, RED);

    // Keep the report details card and navigation button inside the visible
    // client area. The previous layout stretched the card to the full
    // window height and placed the button at screenHeight - 58, which can
    // fall below the visible area on a desktop window with decorations.
    float panelY = 250.0f;
    constexpr float backButtonHeight = 44.0f;
    constexpr float bottomMargin = 18.0f;
    constexpr float panelButtonGap = 16.0f;

    float backButtonY = screenHeight - bottomMargin - backButtonHeight;

    float availablePanelHeight =
        backButtonY - panelY - panelButtonGap;

    float panelHeight = std::clamp(
        availablePanelHeight,
        260.0f,
        400.0f
    );

    DrawRectangleRounded(
        Rectangle{60, panelY, screenWidth - 120.0f, panelHeight},
        0.04f,
        10,
        PANEL
    );

    float col1X = 85.0f;
    float col2X = screenWidth / 2.0f + 15.0f;
    float rowY   = panelY + 16.0f;
    float lineH  = 22.0f;
    float sectionGap = 10.0f;

    char buf[128];

    DrawText("Population", (int)col1X, (int)rowY, 17, TEXT);
    rowY += lineH;

    std::snprintf(buf, sizeof(buf), "Peak Prey:           %d", stats.peakPrey);
    DrawText(buf, (int)col1X, (int)rowY, 15, MUTED);
    rowY += lineH;

    std::snprintf(buf, sizeof(buf), "Peak Predators:   %d", stats.peakPredators);
    DrawText(buf, (int)col1X, (int)rowY, 15, MUTED);
    rowY += lineH;

    std::snprintf(buf, sizeof(buf), "Prey Born:           %d", stats.preyBorn);
    DrawText(buf, (int)col1X, (int)rowY, 15, MUTED);
    rowY += lineH;

    std::snprintf(buf, sizeof(buf), "Predators Born:   %d", stats.predatorBorn);
    DrawText(buf, (int)col1X, (int)rowY, 15, MUTED);
    rowY += lineH + sectionGap;

    DrawText("Deaths", (int)col1X, (int)rowY, 17, TEXT);
    rowY += lineH;

    std::snprintf(buf, sizeof(buf), "Prey  -  Starved: %d   Predated: %d   Old Age: %d",
        stats.preyStarved, stats.preyPredated, stats.preyOldAge);
    DrawText(buf, (int)col1X, (int)rowY, 15, MUTED);
    rowY += lineH;

    std::snprintf(buf, sizeof(buf), "Predators  -  Starved: %d   Old Age: %d",
        stats.predatorStarved, stats.predatorOldAge);
    DrawText(buf, (int)col1X, (int)rowY, 15, MUTED);
    rowY += lineH + sectionGap;

    DrawText("Food & Hunts", (int)col1X, (int)rowY, 17, TEXT);
    rowY += lineH;

    std::snprintf(buf, sizeof(buf), "Food Created:      %d", stats.foodCreated);
    DrawText(buf, (int)col1X, (int)rowY, 15, MUTED);
    rowY += lineH;

    std::snprintf(buf, sizeof(buf), "Food Consumed:  %d", stats.foodConsumed);
    DrawText(buf, (int)col1X, (int)rowY, 15, MUTED);
    rowY += lineH;

    std::snprintf(buf, sizeof(buf), "Prey Food Eaten:  %d", stats.preyFoodEaten);
    DrawText(buf, (int)col1X, (int)rowY, 15, MUTED);
    rowY += lineH;

    std::snprintf(buf, sizeof(buf), "Successful Hunts: %d    Failed: %d",
        stats.successfulHunts, stats.failedHunts);
    DrawText(buf, (int)col1X, (int)rowY, 15, MUTED);

    float col2Y = panelY + 16.0f;
    float traitColW = screenWidth / 2.0f - 100.0f;

    DrawText("Prey Trait Evolution", (int)col2X, (int)col2Y, 17, TEXT);
    col2Y += lineH;

    drawTraitRow(col2X, col2Y, traitColW, "Speed",
        stats.initialPreyTraits.speed,   stats.finalPreyTraits.speed);   col2Y += lineH;
    drawTraitRow(col2X, col2Y, traitColW, "Vision",
        stats.initialPreyTraits.vision,  stats.finalPreyTraits.vision);  col2Y += lineH;
    drawTraitRow(col2X, col2Y, traitColW, "Stamina",
        stats.initialPreyTraits.stamina, stats.finalPreyTraits.stamina); col2Y += lineH;
    drawTraitRow(col2X, col2Y, traitColW, "Escape",
        stats.initialPreyTraits.escape,  stats.finalPreyTraits.escape);  col2Y += lineH + sectionGap + 4.0f;

    DrawText("Predator Trait Evolution", (int)col2X, (int)col2Y, 17, TEXT);
    col2Y += lineH;

    drawTraitRow(col2X, col2Y, traitColW, "Speed",
        stats.initialPredatorTraits.speed,   stats.finalPredatorTraits.speed);   col2Y += lineH;
    drawTraitRow(col2X, col2Y, traitColW, "Vision",
        stats.initialPredatorTraits.vision,  stats.finalPredatorTraits.vision);  col2Y += lineH;
    drawTraitRow(col2X, col2Y, traitColW, "Stamina",
        stats.initialPredatorTraits.stamina, stats.finalPredatorTraits.stamina); col2Y += lineH;
    drawTraitRow(col2X, col2Y, traitColW, "Hunting",
        stats.initialPredatorTraits.hunting, stats.finalPredatorTraits.hunting);

    Rectangle backBtn{
        (float)(screenWidth / 2 - 110),
        backButtonY,
        220.0f,
        backButtonHeight
    };

    if (drawButton(backBtn, "Back to Menu"))
    {
        currentScreen = Screen::Menu;
    }
}

int main(){
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1366, 700, "Evolution Simulator");
    SetTargetFPS(60);

    const int virtualW = 1366;
    const int virtualH = 700;

    RenderTexture2D target = LoadRenderTexture(virtualW, virtualH);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    SimulationConfig config;
    Simulation simulation;

    Screen currentScreen = Screen::Menu;
    bool runningExperiment = false;
    int selectedCreatureId = -1;
    int simulationSpeed = 1;
    bool paused = false;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_F11) || IsKeyPressed(KEY_F))
        {
            ToggleFullscreen();
        }

        int realW = GetScreenWidth();
        int realH = GetScreenHeight();

        float scaleX = (float)realW / virtualW;
        float scaleY = (float)realH / virtualH;
        float scale  = (scaleX < scaleY) ? scaleX : scaleY;

        float offsetX = (realW - virtualW * scale) * 0.5f;
        float offsetY = (realH - virtualH * scale) * 0.5f;

        SetMouseOffset(-(int)offsetX, -(int)offsetY);
        SetMouseScale(1.0f / scale, 1.0f / scale);

        if (currentScreen == Screen::Simulation && !paused)
        {
            float dt = GetFrameTime();
            for (int i = 0; i < simulationSpeed; i++)
            {
                simulation.update(dt);
            }
        }

        BeginTextureMode(target);

        if (currentScreen == Screen::Menu)
        {
            drawMenu(config, currentScreen, simulation, runningExperiment);
        }
        else if (currentScreen == Screen::Simulation)
        {
            drawSimulation(simulation, currentScreen, selectedCreatureId, simulationSpeed, paused);
        }
        else if (currentScreen == Screen::Results)
        {
            drawResults(simulation, currentScreen, paused);
        }

        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);

        Rectangle src{0, 0, (float)virtualW, -(float)virtualH};
        Rectangle dst{offsetX, offsetY, virtualW * scale, virtualH * scale};
        DrawTexturePro(target.texture, src, dst, {0, 0}, 0.0f, WHITE);

        EndDrawing();
    }

    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}