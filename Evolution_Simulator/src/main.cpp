#include "simulation.hpp"
#include "raylib.h"

enum class Screen{
    Menu,Simulation,Results
};

bool drawButton(Rectangle button,const char* text)
{
    Vector2 mousePosition =
        GetMousePosition();

    bool mouseIsOverButton =
        CheckCollisionPointRec(
            mousePosition,
            button
        );

    Color buttonColor = GRAY;

    if (mouseIsOverButton)
    {
        buttonColor = LIGHTGRAY;
    }

    DrawRectangleRec(
        button,
        buttonColor
    );

    DrawRectangleLinesEx(
        button,
        2.0f,
        DARKGRAY
    );

    int textWidth =
        MeasureText(text, 20);

    int textX =
        static_cast<int>(
            button.x
            + (button.width - textWidth)
            / 2.0f
        );

    int textY =
        static_cast<int>(
            button.y
            + (button.height - 20)
            / 2.0f
        );

    DrawText(
        text,
        textX,
        textY,
        20,
        BLACK
    );

    return (
        mouseIsOverButton
        &&
        IsMouseButtonPressed(
            MOUSE_LEFT_BUTTON
        )
    );
}
void drawCounterRow(
    const char* label,
    int& value,
    int minimumValue,
    int maximumValue,
    int y
)
{
    DrawText(
        label,
        250,
        y + 8,
        24,
        BLACK
    );

    Rectangle minusButton =
    {
        500,
        static_cast<float>(y),
        50,
        40
    };

    Rectangle plusButton =
    {
        650,
        static_cast<float>(y),
        50,
        40
    };

    if (drawButton(minusButton, "-"))
    {
        value--;
    }

    if (drawButton(plusButton, "+"))
    {
        value++;
    }

    if (value < minimumValue)
    {
        value = minimumValue;
    }

    if (value > maximumValue)
    {
        value = maximumValue;
    }

    DrawText(
        TextFormat("%d", value),
        575,
        y + 8,
        24,
        BLACK
    );
}
bool drawMenu(
    int& preyCount,
    int& predatorCount,
    int& foodCount,
    int& generationCount
)
{
    ClearBackground(RAYWHITE);

    DrawText(
        "EVOLUTION SIMULATOR",
        245,
        60,
        40,
        DARKBLUE
    );

    DrawText(
        "Choose the starting conditions",
        330,
        115,
        20,
        DARKGRAY
    );

    drawCounterRow(
        "Prey",
        preyCount,
        1,
        100,
        190
    );

    drawCounterRow(
        "Predators",
        predatorCount,
        0,
        50,
        250
    );

    drawCounterRow(
        "Food",
        foodCount,
        1,
        200,
        310
    );

    drawCounterRow(
        "Generations",
        generationCount,
        1,
        100,
        370
    );

    DrawText(
        "Black = Prey",
        300,
        455,
        20,
        DARKGRAY
    );

    DrawText(
        "Red = Predator",
        500,
        455,
        20,
        DARKGRAY
    );

    DrawText(
        "Green = Food",
        380,
        490,
        20,
        DARKGRAY
    );

    Rectangle simulateButton =
    {
        390,
        535,
        220,
        60
    };

    return drawButton(
        simulateButton,
        "SIMULATE"
    );
}
void drawSimulationScreen(
    const Simulation& simulation
)
{
    ClearBackground(RAYWHITE);

    DrawRectangleLines(
        0,
        0,
        static_cast<int>(WORLD_WIDTH),
        static_cast<int>(WORLD_HEIGHT),
        DARKGRAY
    );

    simulation.draw();

    int panelX =
        static_cast<int>(WORLD_WIDTH)
        + 25;

    int preyCount = 0;
    int predatorCount = 0;

    for (
        const Creature& creature
        : simulation.creatures
    )
    {
        if (
            creature.type
            == CreatureType::Prey
        )
        {
            preyCount++;
        }
        else
        {
            predatorCount++;
        }
    }

    DrawText(
        TextFormat(
            "Generation: %d / %d",
            simulation.generation,
            simulation.maxGenerations
        ),
        panelX,
        50,
        20,
        BLACK
    );

    DrawText(
        TextFormat(
            "Prey: %d",
            preyCount
        ),
        panelX,
        100,
        20,
        BLACK
    );

    DrawText(
        TextFormat(
            "Predators: %d",
            predatorCount
        ),
        panelX,
        135,
        20,
        BLACK
    );

    DrawText(
        TextFormat(
            "Food: %d",
            static_cast<int>(
                simulation.environment.foods.size()
            )
        ),
        panelX,
        170,
        20,
        BLACK
    );

    DrawText(
        TextFormat(
            "Food eaten: %d",
            simulation.foodEaten
        ),
        panelX,
        240,
        18,
        DARKGREEN
    );

    DrawText(
        TextFormat(
            "Prey eaten: %d",
            simulation.preyEaten
        ),
        panelX,
        270,
        18,
        RED
    );

    DrawText(
        TextFormat(
            "Prey born: %d",
            simulation.preyBorn
        ),
        panelX,
        320,
        18,
        BLACK
    );

    DrawText(
        TextFormat(
            "Predators born: %d",
            simulation.predatorBorn
        ),
        panelX,
        350,
        18,
        BLACK
    );

    DrawText(
        TextFormat(
            "Time: %.1f / %.1f",
            simulation.generationTimer,
            GENERATION_LENGTH
        ),
        panelX,
        410,
        18,
        DARKGRAY
    );

    DrawText(
        "Simulation running...",
        panelX,
        480,
        18,
        DARKBLUE
    );
}
bool drawResults(
    const Simulation& simulation
)
{
    ClearBackground(RAYWHITE);

    int finalPrey = 0;
    int finalPredators = 0;

    for (
        const Creature& creature
        : simulation.creatures
    )
    {
        if (
            creature.type
            == CreatureType::Prey
        )
        {
            finalPrey++;
        }
        else
        {
            finalPredators++;
        }
    }

    int finalPopulation =
        finalPrey
        + finalPredators;

    int startingPopulation =
        simulation.startingPrey
        + simulation.startingPredators;

    float populationChange =
        0.0f;

    if (startingPopulation > 0)
    {
        populationChange =
            (
                static_cast<float>(
                    finalPopulation
                )
                -
                startingPopulation
            )
            /
            startingPopulation
            * 100.0f;
    }

    const char* outcomeText;

    if (
        finalPrey == 0
        &&
        finalPredators == 0
    )
    {
        outcomeText =
            "Both populations went extinct";
    }
    else if (finalPrey == 0)
    {
        outcomeText =
            "Prey went extinct";
    }
    else if (finalPredators == 0)
    {
        outcomeText =
            "Predators went extinct";
    }
    else
    {
        outcomeText =
            "Both populations survived";
    }

    DrawText(
        "EXPERIMENT COMPLETE",
        300,
        40,
        35,
        DARKBLUE
    );

    DrawText(
        TextFormat(
            "Generations reached: %d",
            simulation.generationsCompleted
        ),
        170,
        110,
        22,
        BLACK
    );

    DrawText(
        TextFormat(
            "Final prey: %d",
            finalPrey
        ),
        170,
        155,
        22,
        BLACK
    );

    DrawText(
        TextFormat(
            "Final predators: %d",
            finalPredators
        ),
        170,
        190,
        22,
        BLACK
    );

    DrawText(
        TextFormat(
            "Final population: %d",
            finalPopulation
        ),
        170,
        225,
        22,
        BLACK
    );

    DrawText(
        TextFormat(
            "Population change: %.1f%%",
            populationChange
        ),
        170,
        260,
        22,
        BLACK
    );

    DrawText(
        TextFormat(
            "Peak population: %d",
            simulation.peakPopulation
        ),
        170,
        310,
        22,
        BLACK
    );

    DrawText(
        TextFormat(
            "Peak prey: %d",
            simulation.peakPrey
        ),
        170,
        345,
        22,
        BLACK
    );

    DrawText(
        TextFormat(
            "Peak predators: %d",
            simulation.peakPredators
        ),
        170,
        380,
        22,
        BLACK
    );

    DrawText(
        "BIRTHS",
        560,
        110,
        24,
        DARKBLUE
    );

    DrawText(
        TextFormat(
            "Prey born: %d",
            simulation.preyBorn
        ),
        560,
        150,
        20,
        BLACK
    );

    DrawText(
        TextFormat(
            "Predators born: %d",
            simulation.predatorBorn
        ),
        560,
        185,
        20,
        BLACK
    );

    DrawText(
        "DEATHS",
        560,
        240,
        24,
        DARKBLUE
    );

    DrawText(
        TextFormat(
            "Prey deaths: %d",
            simulation.preyDied
        ),
        560,
        280,
        20,
        BLACK
    );

    DrawText(
        TextFormat(
            "Predator deaths: %d",
            simulation.predatorDied
        ),
        560,
        315,
        20,
        BLACK
    );

    DrawText(
        "INTERACTIONS",
        560,
        370,
        24,
        DARKBLUE
    );

    DrawText(
        TextFormat(
            "Food eaten: %d",
            simulation.foodEaten
        ),
        560,
        410,
        20,
        BLACK
    );

    DrawText(
        TextFormat(
            "Prey eaten: %d",
            simulation.preyEaten
        ),
        560,
        445,
        20,
        BLACK
    );

    DrawText(
        "TRAITS",
        170,
        435,
        24,
        DARKBLUE
    );

    DrawText(
        TextFormat(
            "Prey speed: %.1f -> %.1f",
            simulation.initialPreySpeed,
            simulation.finalPreySpeed
        ),
        170,
        475,
        18,
        BLACK
    );

    DrawText(
        TextFormat(
            "Prey vision: %.1f -> %.1f",
            simulation.initialPreyVision,
            simulation.finalPreyVision
        ),
        170,
        505,
        18,
        BLACK
    );

    DrawText(
        TextFormat(
            "Predator speed: %.1f -> %.1f",
            simulation.initialPredatorSpeed,
            simulation.finalPredatorSpeed
        ),
        170,
        535,
        18,
        BLACK
    );

    DrawText(
        TextFormat(
            "Predator vision: %.1f -> %.1f",
            simulation.initialPredatorVision,
            simulation.finalPredatorVision
        ),
        170,
        565,
        18,
        BLACK
    );

    DrawText(
        outcomeText,
        560,
        495,
        20,
        DARKBLUE
    );

    Rectangle runAgainButton =
    {
        390,
        610,
        220,
        60
    };

    return drawButton(
        runAgainButton,
        "RUN AGAIN"
    );
}
int main()
{
    InitWindow(
        1000,
        700,
        "Evolution Simulator"
    );

    SetTargetFPS(60);

    Screen currentScreen =
        Screen::Menu;

    Simulation simulation;

    int preyCount = 15;
    int predatorCount = 5;
    int foodCount = 40;
    int generationCount = 20;

    while (!WindowShouldClose())
    {
        float deltaTime =
            GetFrameTime();

        if (
            currentScreen
            == Screen::Menu
        )
        {
            BeginDrawing();

            bool startSimulation =
                drawMenu(
                    preyCount,
                    predatorCount,
                    foodCount,
                    generationCount
                );

            EndDrawing();

            if (startSimulation)
            {
                simulation.start(
                    preyCount,
                    predatorCount,
                    foodCount,
                    generationCount
                );

                currentScreen =
                    Screen::Simulation;
            }
        }
        else if (
            currentScreen
            == Screen::Simulation
        )
        {
            simulation.update(
                deltaTime
            );

            BeginDrawing();

            drawSimulationScreen(
                simulation
            );

            EndDrawing();

            if (simulation.finished)
            {
                currentScreen =
                    Screen::Results;
            }
        }
        else
        {
            BeginDrawing();

            bool runAgain =
                drawResults(
                    simulation
                );

            EndDrawing();

            if (runAgain)
            {
                currentScreen =
                    Screen::Menu;
            }
        }
    }

    CloseWindow();

    return 0;
}