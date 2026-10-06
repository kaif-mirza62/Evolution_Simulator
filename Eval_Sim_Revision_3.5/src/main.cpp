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
        (int)(x + width - 150),
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
        (int)(x + width - 65),
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
        Rectangle{panelX, panelY, panelWidth, 425.0f},
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

    drawValueControl(
        Rectangle{left, panelY + 180, controlWidth, 78},
        "Food",
        config.initialFood,
        20,
        500,
        20,
        config.initialFood
    );

    drawValueControl(
        Rectangle{left + controlWidth + 10, panelY + 180, controlWidth, 78},
        "Generations",
        config.generation,1,100,5,config.generation
    );

    Rectangle startButton{
    panelX + 30,
    panelY + 300,
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
        "F11  Fullscreen     1-5  Simulation speed     SPACE  Pause",
        70,
        screenHeight - 45,
        16,
        MUTED
    );
}

void drawSimulation(
    Simulation& simulation,
    Screen& currentScreen,
    int& selectedCreatureId,
    int& simulationSpeed,
    bool& paused
)
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    const int panelWidth = std::min(310, screenWidth / 3);

    float worldWidth = (int)(screenWidth - panelWidth);
    float worldHeight = (int)(screenHeight);

    simulation.setWorldSize(worldWidth, worldHeight);

    simulation.draw(worldWidth, worldHeight);

    DrawRectangle(
        (int)(worldWidth),
        0,
        panelWidth,
        screenHeight,
        Color{250, 251, 250, 255}
    );

    DrawText(
        "LIVE ECOSYSTEM",
        (int)(worldWidth + 22),
        22,
        25,
        TEXT
    );

    char generationText[64];

    std::snprintf(
        generationText,
        sizeof(generationText),
        "Generation %d / %d",
        simulation.getGeneration(),
        simulation.getGenerationCount()
    );

    DrawText(
        generationText,
        (int)(worldWidth + 22),
        61,
        18,
        MUTED
    );

    Rectangle progressBar{
        worldWidth + 22,
        88,
        panelWidth - 44.0f,
        12
    };

    DrawRectangleRounded(progressBar, 0.5f, 8, Color{220, 225, 220, 255});

    progressBar.width *= simulation.getGenerationProgress();

    DrawRectangleRounded(progressBar, 0.5f, 8, ACCENT);

    char preyText[32];
    char predatorText[32];
    char foodText[32];

    std::snprintf(preyText, sizeof(preyText), "%d", simulation.getPreyCount());
    std::snprintf(predatorText, sizeof(predatorText), "%d", simulation.getPredatorCount());
    std::snprintf(foodText, sizeof(foodText), "%d", simulation.getFoodCount());

    drawStatCard(worldWidth + 18, 120, panelWidth - 36, 72, "Prey", preyText, PREY_COLOR);
    drawStatCard(worldWidth + 18, 201, panelWidth - 36, 72, "Predators", predatorText, PREDATOR_COLOR);
    drawStatCard(worldWidth + 18, 282, panelWidth - 36, 72, "Food", foodText, FOOD_COLOR);


    DrawText(
        "SIMULATION SPEED",
        (int)(worldWidth + 22),
        375,
        16,
        MUTED
    );

    float speedButtonWidth = (panelWidth - 44.0f - 16.0f) / 5.0f;

    for (int i = 1; i <= 5; i++)
    {
        Rectangle speedButton{
            worldWidth + 22 + (i - 1) * (speedButtonWidth + 4),
            405,
            speedButtonWidth,
            38
        };

        bool active = simulationSpeed == i;

        Color buttonColor = active ? ACCENT : PANEL_DARK;

        DrawRectangleRounded(
            speedButton,
            0.18f,
            8,
            buttonColor
        );

        char text[8];
        std::snprintf(text, sizeof(text), "%dx", i);

        int textWidth = MeasureText(text, 16);

        DrawText(
            text,
            (int)(speedButton.x + (speedButton.width - textWidth) / 2),
            (int)(speedButton.y + 10),
            16,
            RAYWHITE
        );

        if (CheckCollisionPointRec(GetMousePosition(), speedButton) &&
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            
            
        }
    }

    Rectangle pauseButton{
        worldWidth + 22,
        460,
        panelWidth - 44.0f,
        45
    };

    const char* pauseText = paused ? "RESUME" : "PAUSE";

    if (drawButton(pauseButton, pauseText))
    {
        paused = !paused;
    }

    DrawText(
        "CLICK A CREATURE",
        (int)(worldWidth + 22),
        527,
        16,
        MUTED
    );

    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 mouse = GetMousePosition();

        if (mouse.x < worldWidth)
        {
            float bestDistance = 20.0f;
            int bestId = -1;

            for (const Creature& creature : simulation.getCreatures())
            {
                float dx = mouse.x - creature.x;
                float dy = mouse.y - creature.y;
                float distance = std::sqrt(dx * dx + dy * dy);

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

    if (selected != nullptr)
    {
        
        DrawCircleLines(
            (int)(selected->x),
            (int)(selected->y),
            13.0f,
            ACCENT
        );

        char line[100];

        const char* typeName =
            selected->type == CreatureType::Prey
                ? "PREY"
                : "PREDATOR";

        DrawText(
            typeName,
            (int)(worldWidth + 22),
            555,
            22,
            selected->type == CreatureType::Prey
                ? TEXT
                : RED
        );

        std::snprintf(line, sizeof(line), "ID: %d", selected->id);
        DrawText(line, (int)(worldWidth + 22), 588, 16, MUTED);

        const char* actionName = "Wander";

        if (selected->brain.currentAction == Action::FindFood)
            actionName = "Finding food";
        else if (selected->brain.currentAction == Action::Escape)
            actionName = "Escaping";
        else if (selected->brain.currentAction == Action::Hunt)
            actionName = "Hunting";
        else if (selected->brain.currentAction == Action::Rest)
            actionName = "Resting";

        std::snprintf(line, sizeof(line), "Brain: %s", actionName);
        DrawText(line, (int)(worldWidth + 22), 610, 16, TEXT);

        std::snprintf(line, sizeof(line), "Energy: %.0f", selected->energy);
        DrawText(line, (int)(worldWidth + 22), 632, 16, TEXT);

        std::snprintf(line, sizeof(line), "Speed: %.1f  Vision: %.0f", selected->speed, selected->vision);
        DrawText(line, (int)(worldWidth + 22), 654, 15, MUTED);
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
        45,
        42,
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
        Rectangle{60, 105, screenWidth - 120.0f, 80},
        0.06f,
        10,
        PANEL
    );

    DrawText(
        outcome,
        85,
        124,
        28,
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
        158,
        16,
        MUTED
    );
    
    float cardWidth = (screenWidth - 150.0f) / 3.0f;

    char text[32];

    std::snprintf(text, sizeof(text), "%d -> %d", stats.initialPrey, stats.finalPrey);
    drawStatCard(60, 210, cardWidth, 85, "Prey", text, BLACK);

    std::snprintf(text, sizeof(text), "%d -> %d", stats.initialPredators, stats.finalPredators);
    drawStatCard(75 + cardWidth, 210, cardWidth, 85, "Predators", text, RED);

    std::snprintf(text, sizeof(text), "%d", stats.successfulHunts);
    drawStatCard(90 + cardWidth * 2, 210, cardWidth, 85, "Successful hunts", text, RED);

    
    
    

    float panelY = 325.0f;
    float panelHeight = screenHeight - panelY - 90.0f;

    DrawRectangleRounded(
        Rectangle{60, panelY, screenWidth - 120.0f, panelHeight},
        0.04f,
        10,
        PANEL
    );

    // ── Section headers ───────────────────────────────────────────────────────
    float col1X = 85.0f;
    float col2X = screenWidth / 2.0f + 15.0f;
    float rowY   = panelY + 22.0f;

    // ── Left column: Population & Deaths ─────────────────────────────────────
    DrawText("Population", (int)col1X, (int)rowY, 19, TEXT);
    rowY += 30.0f;

    char buf[128];

    std::snprintf(buf, sizeof(buf), "Peak Prey:           %d", stats.peakPrey);
    DrawText(buf, (int)col1X, (int)rowY, 17, MUTED);
    rowY += 26.0f;

    std::snprintf(buf, sizeof(buf), "Peak Predators:   %d", stats.peakPredators);
    DrawText(buf, (int)col1X, (int)rowY, 17, MUTED);
    rowY += 26.0f;

    std::snprintf(buf, sizeof(buf), "Prey Born:           %d", stats.preyBorn);
    DrawText(buf, (int)col1X, (int)rowY, 17, MUTED);
    rowY += 26.0f;

    std::snprintf(buf, sizeof(buf), "Predators Born:   %d", stats.predatorBorn);
    DrawText(buf, (int)col1X, (int)rowY, 17, MUTED);
    rowY += 38.0f;

    DrawText("Deaths", (int)col1X, (int)rowY, 19, TEXT);
    rowY += 30.0f;

    std::snprintf(buf, sizeof(buf), "Prey  –  Starved: %d   Predated: %d   Old Age: %d",
        stats.preyStarved, stats.preyPredated, stats.preyOldAge);
    DrawText(buf, (int)col1X, (int)rowY, 17, MUTED);
    rowY += 26.0f;

    std::snprintf(buf, sizeof(buf), "Predators  –  Starved: %d   Old Age: %d",
        stats.predatorStarved, stats.predatorOldAge);
    DrawText(buf, (int)col1X, (int)rowY, 17, MUTED);
    rowY += 38.0f;

    DrawText("Food & Hunts", (int)col1X, (int)rowY, 19, TEXT);
    rowY += 30.0f;

    std::snprintf(buf, sizeof(buf), "Food Created:      %d", stats.foodCreated);
    DrawText(buf, (int)col1X, (int)rowY, 17, MUTED);
    rowY += 26.0f;

    std::snprintf(buf, sizeof(buf), "Food Consumed:  %d", stats.foodConsumed);
    DrawText(buf, (int)col1X, (int)rowY, 17, MUTED);
    rowY += 26.0f;

    std::snprintf(buf, sizeof(buf), "Prey Food Eaten:  %d", stats.preyFoodEaten);
    DrawText(buf, (int)col1X, (int)rowY, 17, MUTED);
    rowY += 26.0f;

    std::snprintf(buf, sizeof(buf), "Successful Hunts: %d    Failed: %d",
        stats.successfulHunts, stats.failedHunts);
    DrawText(buf, (int)col1X, (int)rowY, 17, MUTED);

    // ── Right column: Trait Evolution ─────────────────────────────────────────
    float col2Y = panelY + 22.0f;
    float traitColW = screenWidth / 2.0f - 100.0f;

    DrawText("Prey Trait Evolution", (int)col2X, (int)col2Y, 19, TEXT);
    col2Y += 30.0f;

    drawTraitRow(col2X, col2Y, traitColW, "Speed",
        stats.initialPreyTraits.speed,   stats.finalPreyTraits.speed);   col2Y += 28.0f;
    drawTraitRow(col2X, col2Y, traitColW, "Vision",
        stats.initialPreyTraits.vision,  stats.finalPreyTraits.vision);  col2Y += 28.0f;
    drawTraitRow(col2X, col2Y, traitColW, "Stamina",
        stats.initialPreyTraits.stamina, stats.finalPreyTraits.stamina); col2Y += 28.0f;
    drawTraitRow(col2X, col2Y, traitColW, "Escape",
        stats.initialPreyTraits.escape,  stats.finalPreyTraits.escape);  col2Y += 44.0f;

    DrawText("Predator Trait Evolution", (int)col2X, (int)col2Y, 19, TEXT);
    col2Y += 30.0f;

    drawTraitRow(col2X, col2Y, traitColW, "Speed",
        stats.initialPredatorTraits.speed,   stats.finalPredatorTraits.speed);   col2Y += 28.0f;
    drawTraitRow(col2X, col2Y, traitColW, "Vision",
        stats.initialPredatorTraits.vision,  stats.finalPredatorTraits.vision);  col2Y += 28.0f;
    drawTraitRow(col2X, col2Y, traitColW, "Stamina",
        stats.initialPredatorTraits.stamina, stats.finalPredatorTraits.stamina); col2Y += 28.0f;
    drawTraitRow(col2X, col2Y, traitColW, "Hunting",
        stats.initialPredatorTraits.hunting, stats.finalPredatorTraits.hunting);

    // ── Back to Menu button ───────────────────────────────────────────────────
    Rectangle backBtn{
        (float)(screenWidth / 2 - 110),
        (float)(screenHeight - 72),
        220.0f,
        48.0f
    };

    if (drawButton(backBtn, "Back to Menu"))
    {
        currentScreen = Screen::Menu;
    }
}

int main()
{
    InitWindow(1280, 720, "Evolution Simulator");
    SetTargetFPS(60);

    SimulationConfig config;
    Simulation simulation;

    Screen currentScreen = Screen::Menu;
    bool runningExperiment = false;
    int selectedCreatureId = -1;
    int simulationSpeed = 1;
    bool paused = false;

    while (!WindowShouldClose())
    {
        if (currentScreen == Screen::Simulation && !paused)
        {
            float dt = GetFrameTime();
            for (int i = 0; i < simulationSpeed; i++)
            {
                simulation.update(dt);
            }
        }

        BeginDrawing();

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

        EndDrawing();
    }

    CloseWindow();

    return 0;
}