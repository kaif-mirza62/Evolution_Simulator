# Evolution Lab — Evolution Simulator

> A real-time predator–prey ecosystem simulator where creatures evolve biologically **and** learn behaviourally through Reinforcement Learning.

---

## 1. Technology Stack

| Category | Technology |
|---|---|
| **Language** | C++20 |
| **Build System** | CMake 3.24+ (with `FetchContent`) |
| **Graphics / Windowing** | [raylib 6.0](https://www.raylib.com/) — fetched automatically at build time |
| **Random Number Generation** | `std::mt19937` (Mersenne Twister) from `<random>` |
| **Standard Library** | `<array>`, `<vector>`, `<deque>`, `<cmath>`, `<algorithm>`, `<limits>`, `<string>`, `<cstdio>` |
| **ML / RL** | Custom hand-written Deep Q-Network (DQN) — **no external ML library** |

All machine learning, neural network forward passes, and backpropagation are implemented **from scratch in plain C++** with no third-party ML dependency.

---

## 2. What The Program Does

**Evolution Lab** simulates a living ecosystem containing three kinds of entities: **prey**, **predators**, and **food**.

When you launch the app, you are presented with a setup screen where you choose how many prey, how many predators, and how much food to start with. Once you hit *Start Experiment*, the simulation begins running in real time inside a scrolling 2-D world.

### What happens inside the simulation

Every creature has a set of biological traits — how fast it moves, how far it can see, how much energy it can store, how good it is at hunting or escaping — and a simple brain that reacts to what it currently perceives.

- **Prey** roam the world eating green food pellets to stay alive. If they see a predator, they try to run. If they starve or get caught, they die.
- **Predators** hunt prey for energy. If no prey is visible they wander or rest. If they starve, they die.
- **Food** spawns randomly across the world and is replenished automatically when supplies run low.

Every few seconds the simulation reaches the end of a **generation**. The fittest survivors (those who ate the most, lived the longest, and killed the most, in the case of predators) are chosen as parents. They produce offspring whose traits are a blend of their parents' traits with a small random mutation applied. Over many generations the population's traits drift in directions that make them more successful — this is the **evolutionary** part.

At the same time, an independent **Reinforcement Learning agent** watches every creature's experience and continuously learns which actions work best in each situation. The neural network figures out, for example, that a prey creature with very low energy near a predator should escape rather than search for food. This learning carries across generations — the neural networks are never reset.

### Screens

| Screen | Description |
|---|---|
| **Menu** | Configure initial prey, predator, and food counts, then start the experiment |
| **Simulation** | Watch the live ecosystem; inspect individual creatures; control speed (1×–5×) and pause |
| **Results** | After the experiment ends, see population stats, cause-of-death breakdown, and trait evolution (initial → final values) |

---

## 3. File Structure

```
Eval_Sim_Revision_V_0.5.2/
│
├── CMakeLists.txt          # Build configuration
│
├── assets/
│   ├── environment.png     # Background texture for the world
│   ├── food.png            # Sprite for food pellets
│   ├── prey.png            # Sprite for prey creatures
│   └── predator.png        # Sprite for predator creatures
│
├── include/                # Header files (declarations)
│   ├── types.hpp
│   ├── brain.hpp
│   ├── creature.hpp
│   ├── environment.hpp
│   ├── neural_network.hpp
│   ├── replay_buffer.hpp
│   ├── rl_agent.hpp
│   └── simulation.hpp
│
└── src/                    # Source files (implementations)
    ├── main.cpp
    ├── brain.cpp
    ├── creature.cpp
    ├── environment.cpp
    ├── neural_network.cpp
    ├── replay_buffer.cpp
    ├── rl_agent.cpp
    └── simulation.cpp
```

---

### Header Files (`include/`)

#### `types.hpp`
The central definitions file. Defines the two core enumerations used everywhere:

- **`CreatureType`** — `Prey` or `Predator`.
- **`Action`** — the five possible actions a creature can take: `Wander`, `FindFood`, `Escape`, `Hunt`, `Rest`.
- **`DeathCause`** — `None`, `Starvation`, `Predation`, or `OldAge`.
- **`LearningState`** — a fixed-size array of 30 floats that encodes everything the RL agent can see about the world at one moment in time.
- **`Experience`** — one memory record used for RL training: the state before an action, which action was taken, the reward received, the state after, and whether the creature died.

---

#### `brain.hpp`
Declares the `Brain` struct — the **instinct layer** of a creature. It holds:

- The creature's current action.
- A wander angle that drifts randomly over time.
- A decision timer so decisions are not made every single frame.

The one method, `decide(...)`, looks at simple boolean signals (is the creature hungry? does it see food? does it see danger?) and returns a hard-coded rule-based action. This becomes the *instinct suggestion* that the RL agent may choose to follow or override.

---

#### `creature.hpp`
Declares the `Creature` struct — the complete data record for one living entity. Fields include:

- **Identity** — `id`, `type`.
- **Position** — `x`, `y`.
- **Biological traits** — `speed`, `vision`, `stamina`, `hunting`, `escape`. These are inherited and mutated across generations.
- **Vitals** — `energy`, `age`, `maxAge`, `alive`, `deathCause`.
- **Performance counters** — `foodEaten`, `kills`. Used to compute fitness for reproduction selection.
- **Brain** — an embedded `Brain` instance for instinct decisions.
- **RL bookkeeping** — `learningState`, `learningInstinct`, `learningAction`, `learningReward`, `learningTransitionPending`. These fields allow the simulation to defer saving an experience until the *next* frame, when the resulting next-state is known.

---

#### `environment.hpp`
Declares the `Food` struct (a position and an energy value) and the `Environment` class, which owns the list of all food items currently on the map, plus the background texture. Provides methods to spawn food randomly across the world, load/unload the background texture, and draw itself.

---

#### `neural_network.hpp`
Declares `NeuralNetwork` — the core learnable model. Architecture:

| Layer | Size |
|---|---|
| Input | 30 neurons (`LEARNING_STATE_SIZE`) |
| Hidden 1 | 32 neurons |
| Hidden 2 | 32 neurons |
| Output | 5 neurons (one Q-value per action) |

Weight arrays (`w1`, `w2`, `w3`) and bias arrays (`b1`, `b2`, `b3`) are stored as flat `std::array<float, N>` buffers.

Key methods:
- `predict(state)` — forward pass, returns an `Output` array of 5 Q-values.
- `trainBatch(batch, learningRate)` — one SGD update over a batch of `(state, target Q-values)` pairs; returns average loss.
- `copyFrom(other)` — hard copies all weights, used to periodically sync the target network.

---

#### `replay_buffer.hpp`
Declares `ReplayBuffer` — a fixed-capacity circular memory that stores `Experience` records. When it is full the oldest experience is discarded. Provides:

- `add(experience)` — store one experience.
- `sample(batchSize, rng)` — pick a random subset of stored experiences for training.
- `size()` / `clear()`.

---

#### `rl_agent.hpp`
Declares `RLAgent` — the full DQN agent. It owns:

- An **online network** (trained every step).
- A **target network** (frozen copy, used to compute stable target Q-values).
- A `ReplayBuffer`.
- An epsilon value for exploration.

Hyperparameters embedded as compile-time constants:

| Constant | Value | Meaning |
|---|---|---|
| `BUFFER_CAPACITY` | 10 000 | Maximum experiences stored |
| `BATCH_SIZE` | 64 | Experiences sampled per training step |
| `WARMUP_EXPERIENCES` | 512 | Experiences collected before training starts |
| `TARGET_UPDATE_STEPS` | 250 | Training steps between target network syncs |
| `EPSILON_DECAY_STEPS` | 20 000 | Steps to decay epsilon from 1.0 to 0.05 |
| `LEARNING_RATE` | 0.0005 | SGD learning rate |
| `GAMMA` | 0.97 | Discount factor for future rewards |
| `INITIAL_EPSILON` | 1.0 | Start fully random |
| `MIN_EPSILON` | 0.05 | Never go fully greedy |
| `INSTINCT_BIAS` | 0.20 | Bonus added to the instinct action's Q-value |

Key methods:
- `chooseAction(state, instinct)` — epsilon-greedy action selection with instinct bias.
- `remember(experience)` — store one experience.
- `train()` — sample a batch, compute DQN targets, run a training step.
- `reset()` — reinitialise networks and buffer (called at the start of each new experiment).

---

#### `simulation.hpp`
The largest and most central header. Declares:

- **`SimulationConfig`** — starting counts for prey, predators, and food.
- **`TraitMeans`** — average speed, vision, stamina, hunting, escape across a population. Recorded at the start and end of the experiment to show evolutionary drift.
- **`SimulationStats`** — a complete statistics record: initial/final/peak populations, births, deaths by cause, food consumed, successful and failed hunts, completed generations, and initial/final trait means for both populations.
- **`Simulation`** — the main simulation class. Owns the creature list, environment, random number generator, and one `RLAgent` each for prey and predators. Exposes the full public API used by `main.cpp` (start, update, draw, getters for UI display, etc.). The private `Perception` struct and `buildLearningState` method are the bridge between the simulation world and the RL system.

---

### Source Files (`src/`)

#### `main.cpp`
The entry point and the entire UI layer. Runs the raylib game loop and manages three screens: Menu, Simulation, and Results.

- Draws the styled start screen with `+`/`-` controls to configure the experiment.
- Draws the live simulation screen: world view on the left, sidebar on the right with population counters, a generation progress bar, the learning diagnostics panel (memory size, training steps, epsilon, loss), speed buttons, pause/resume, and a creature inspector that shows stats when you click on a creature.
- Draws the results screen after the experiment ends: stat cards for populations, cause-of-death breakdown, and a trait evolution table showing how each biological trait changed from generation 1 to the final generation.
- Handles keyboard shortcuts: `F11` for fullscreen, `1`–`5` for simulation speed, `Space` to pause.

---

#### `brain.cpp`
Implements `Brain::decide(...)`. A simple rule-based system:

- **Prey** — if a predator is visible, escape. Otherwise if hungry and food is visible, find food. Otherwise wander.
- **Predator** — if prey is visible, hunt. If not hungry, rest. Otherwise wander.

The wander angle is updated periodically with a small random perturbation so creatures do not walk in straight lines.

---

#### `creature.cpp`
Currently a minimal stub that includes the necessary headers. All creature logic lives inside `simulation.cpp` which directly operates on `Creature` structs.

---

#### `environment.cpp`
Implements food spawning (random positions with a small border padding), background texture loading/unloading, and world rendering via raylib's `DrawTexturePro` to scale the background image to the current world dimensions.

---

#### `neural_network.cpp`
Implements the full forward pass and training pass of the neural network from scratch.

- **Forward pass (`predict`)** — matrix-vector multiply + bias add, then ReLU activation for hidden layers; linear (no activation) for the output layer.
- **Training (`trainBatch`)** — full backpropagation with Huber loss (smooth L1). Gradients are accumulated over the batch, averaged, clamped to ±5 (gradient clipping), then applied with vanilla SGD.
- **Weight initialisation** — He (Kaiming) initialisation: weights drawn from `N(0, sqrt(2/fanIn))` for each layer, appropriate for ReLU activations.

---

#### `replay_buffer.cpp`
Implements the circular experience buffer using a `std::deque`. When capacity is exceeded, `pop_front()` removes the oldest entry. `sample()` picks entries uniformly at random (with replacement).

---

#### `rl_agent.cpp`
Implements the DQN training loop:

- During warmup it mostly follows the instinct and occasionally acts randomly.
- After warmup it switches to epsilon-greedy selection using the online network's Q-values, with a +0.20 bias added to the instinct action's score.
- Each training call samples 64 experiences, builds DQN targets (Bellman equation), and calls `trainBatch` on the online network.
- Every 250 training steps the target network is hard-copied from the online network.
- Epsilon decays linearly from 1.0 to 0.05 over 20 000 training steps.

---

#### `simulation.cpp`
The heart of the program (~1 800 lines). Responsibilities:

1. **Initialisation** — spawn creatures with randomised traits, scatter food, record initial trait means.
2. **Per-frame update** — for every living creature: age it, run perception, build learning state, process pending RL experience, choose action via RL agent, execute the action (movement, eating, hunting, resting, wandering), deduct energy costs, check for death, compute and store the reward signal.
3. **RL integration** — after each action the reward is computed and stored on the creature. On the *next* frame the previous `(state, action, reward)` is combined with the new `state` to form a complete `Experience` that is pushed to the replay buffer. If the creature died, the terminal experience is committed immediately with a −10 penalty.
4. **Training** — `preyRL.train()` and `predatorRL.train()` are called every frame.
5. **Population management** — dead creatures are removed; food is replenished if it falls below 30 items.
6. **Generation end** — every 6 seconds survivors are ranked by fitness, the top 40% become parents, offspring are produced with averaged + mutated traits, and the generation counter advances.
7. **Experiment end** — triggered when all prey are gone or generation limit is reached; final stats are recorded.

---

## 4. How the ML and RL Work

### Overview

Each creature species (prey and predators) shares **one RL agent**. Every individual creature in that species contributes its experiences to the same shared memory and benefits from the same shared policy. This is a form of **centralised learning with decentralised execution** — the neural network is trained centrally from the experiences of all creatures of that type, but each creature acts independently using its own local observation.

---

### Step 1 — What the agent perceives (the State)

Each frame, a 30-dimensional `LearningState` vector is built for every creature. It encodes, all normalised to the [0, 1] range:

| Indices | What it encodes |
|---|---|
| 0 | Energy as a fraction of max stamina |
| 1 | Age as a fraction of max age |
| 2–6 | Normalised speed, vision, stamina, hunting skill, escape skill |
| 7–10 | Nearest food: distance ratio, direction X, direction Y, visibility flag |
| 11–14 | Nearest predator: distance ratio, direction X, direction Y, visibility flag |
| 15–18 | Nearest prey: distance ratio, direction X, direction Y, visibility flag |
| 19 | Nearby predator count (normalised) |
| 20 | Nearby prey count (normalised) |
| 21–24 | Proximity to each of the four world borders |
| 25–29 | One-hot encoding of the instinct suggestion |

The last five slots are a one-hot vector that tells the network what the rule-based instinct is suggesting. This gives the network a strong prior to start from.

---

### Step 2 — The Instinct Brain (rule-based layer)

Before the RL agent makes its decision, the `Brain::decide()` method produces a simple rule-based suggestion:

- Prey → escape if danger visible, eat if hungry and food visible, otherwise wander.
- Predator → hunt if prey visible, rest if full, otherwise wander.

This instinct is then passed to the RL agent both as part of the state vector and as a direct input to action selection.

---

### Step 3 — Action Selection (Epsilon-Greedy + Instinct Bias)

The RL agent picks an action using this strategy:

1. **Warmup phase** (fewer than 512 experiences): Mostly follow instinct, occasionally pick randomly, to bootstrap the replay buffer.
2. **Normal phase**:
   - With probability **ε (epsilon)**, pick a random valid action (exploration).
   - Otherwise, query the online network for Q-values, add `+0.20` to the instinct action's Q-value, then pick the highest-scoring valid action (exploitation with instinct guidance).

Prey are only allowed to choose from `{Wander, FindFood, Escape}`. Predators from `{Wander, Hunt, Rest}`. This prevents biologically nonsensical actions.

Epsilon starts at 1.0 (fully random) and decays linearly to 0.05 over 20 000 training steps.

---

### Step 4 — Executing the Action

The chosen action changes the creature's movement and energy:

| Action | What happens |
|---|---|
| `FindFood` | Move toward nearest visible food; consume it (+energy) when close enough |
| `Escape` | Move directly away from the nearest predator at boosted speed |
| `Hunt` | Move toward nearest visible prey; attempt a probabilistic kill on contact |
| `Rest` | Stay still; slowly regenerate energy |
| `Wander` | Move in a slowly-drifting random direction at reduced speed |

Energy is drained each frame based on base metabolic cost plus speed. Hunting and escaping cost extra energy.

---

### Step 5 — The Reward Signal

After each action, a reward is computed:

| Event | Reward |
|---|---|
| Surviving one frame | +0.15 × deltaTime (living is mildly rewarded) |
| Energy gained/lost | ±(energy change × 0.03) |
| Eating a food item | +1.0 |
| Killing a prey | +2.0 |
| Escaping (increasing distance from predator) | +0.4 |
| Dying | −10.0 (large penalty, terminal) |

These signals guide the networks to prefer behaviours that keep the creature alive and fed.

---

### Step 6 — Storing the Experience

Because the "next state" is only known on the following frame, storing is deferred:

1. After executing an action the creature stores `(state, action, reward)` on itself and sets `learningTransitionPending = true`.
2. On the **next** frame the fresh state is computed; the pending transition is completed as `(state, action, reward, nextState, done=false)` and pushed into the replay buffer.
3. If the creature **dies**, a terminal experience `(state, action, reward − 10, zeroed nextState, done=true)` is committed immediately.

---

### Step 7 — The Replay Buffer

`ReplayBuffer` stores up to **10 000** experiences in a `std::deque`. When full, the oldest entry is discarded (FIFO). Storing old memories and sampling them randomly are two key tricks that make DQN stable:

- **Experience replay** breaks the temporal correlation between consecutive training samples, which would otherwise cause the network to catastrophically forget earlier lessons.
- **Random sampling** means each batch is an independent, approximately i.i.d. sample from the agent's life history.

---

### Step 8 — Training (Deep Q-Learning / DQN)

Every frame, after all creatures have been updated, `train()` is called on both RL agents. If fewer than 512 experiences are stored, training is skipped (warmup). Otherwise:

1. **Sample** 64 random experiences from the buffer.
2. For each experience, compute the **Bellman target**:

$$Q_{\text{target}} = r + \gamma \cdot \max_{a'} Q_{\text{target}}(s', a') \quad \text{if not terminal}$$

$$Q_{\text{target}} = r \quad \text{if terminal (done = true)}$$

   where $\gamma = 0.97$ (high discount: future rewards are valued almost as much as immediate ones).

3. The target network — a lagged copy of the online network — provides $Q_{\text{target}}(s', a')$. Using a separate frozen network prevents the training target from "chasing itself" and keeps learning stable.

4. Build a training batch: for each experience, take the online network's current Q-value vector and replace only the Q-value for the taken action with the Bellman target. All other action Q-values remain unchanged so backpropagation only adjusts the relevant output neuron.

5. Run **one step of mini-batch SGD** via `trainBatch`:
   - Forward pass through the online network.
   - Compute **Huber loss** (smooth L1): squared error for small errors, absolute error for large errors. This makes training robust to outlier experiences.
   - **Backpropagation** through all three layers using the chain rule and ReLU derivatives.
   - **Gradient clipping** — gradients are clamped to ±5 before applying updates to prevent exploding gradients.
   - **SGD update**: `weight -= learningRate × gradient` with `learningRate = 0.0005`.

6. Every **250 training steps**, hard-copy the online network into the target network (target network update).

---

### Step 9 — Evolution (Biological Layer)

Every 6 seconds a generation ends. Biological evolution runs independently of RL:

1. **Fitness scoring** — each creature earns: `energy + foodEaten × 15 + kills × 30 + age × 2`.
2. **Selection** — the top 40% by fitness become eligible parents.
3. **Reproduction** — offspring traits are the average of two randomly-chosen parents' traits, then perturbed by a small Gaussian mutation (`σ = 8%` of the trait value). All traits are clamped to valid ranges.
4. **Neural networks are NOT reset** — the RL agents keep their learned weights across generations, so learning compounds over time.

Over many generations, traits that correlate with higher fitness (e.g. faster prey that escape predators, prey with better vision to spot food earlier) will be naturally selected for, causing measurable drift visible in the Results screen.

---

### The Two-Layer Intelligence System

```
┌─────────────────────────────────────────────────────────┐
│                     Each Creature                        │
│                                                         │
│  Perception  →  Brain (instinct)  →  Instinct action    │
│       │                                    │            │
│       └────────── State vector (30D) ──────┤            │
│                          │                 │            │
│                    RLAgent.chooseAction()               │
│                 (epsilon-greedy + instinct bias)        │
│                          │                              │
│                    Final action executed                 │
│                          │                              │
│                    Reward computed  →  Experience stored │
│                          │                              │
│               (Every frame) RLAgent.train()             │
│         Online Network updated via DQN + backprop       │
└─────────────────────────────────────────────────────────┘
```

The instinct brain provides a safe, biologically-plausible fallback that is especially important early in training when the RL network is still random. As training progresses and epsilon decays, the RL network increasingly overrides the instinct when it has learned something better — for example, a prey creature might learn to start moving toward food *before* it is technically "hungry" by the instinct's threshold, or a predator might learn to take a more efficient path to intercept prey.

---

## 5. Building and Running

```bash
# Clone / navigate to the project directory
cd Eval_Sim_Revision_V_0.5.2

# Configure (downloads raylib automatically)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run (from project root so assets/ is found)
./build/EvolutionSimulator
```

> **Note:** An internet connection is required on first build so CMake can download raylib 6.0 via `FetchContent`.
