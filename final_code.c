#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define N_COMPONENTS 5
#define N_ITERATIONS 100
#define N_TIMESTEPS 24 * 4

// Defining Grid Model Structure
typedef struct
{
    char *component_state;
    float failure_rate;
    float rating;
    float *outage_duration;
} GridComponent;

// Function to sample demand from the demand model
float sample_demand(int timestep, int iteration, GridComponent *components)
{

    float base_demand = 50.0; // Base demand
    float demand_factor_daily = 1.0 + 0.1 * sin(2 * M_PI * timestep / (24.0 * 2.0) + M_PI / 4.0);           // Daily demand pattern
    float demand_factor_weekly = 1.0 + 0.1 * sin(2 * M_PI * iteration / (N_ITERATIONS * 7.0) + M_PI / 4.0); // Weekly demand pattern
    float demand_factor_random = 1.0 + 0.05 * (float)rand() / RAND_MAX;                                     // Random demand variation
    float demand = base_demand * demand_factor_daily * demand_factor_weekly * demand_factor_random;         // Calculate demand
    for (int i = 0; i < N_COMPONENTS; i++)                                                                  // Apply component-specific demand factors
    {
        demand *= (1.0 + 0.02 * components[i].rating * sin(2 * M_PI * timestep / 24.0 + 2 * M_PI * i / N_COMPONENTS));
    }
    return demand;
}

// Function to sample renewable generation from the renewable generation model
float sample_renewable_generation(int timestep, GridComponent *components)
{

    float base_generation = 50.0;                                                                                       // Base generation
    float generation_factor_daily = 1.0 + 0.1 * sin(2 * M_PI * timestep / 24.0 + M_PI / 4.0);                           // Daily generation pattern
    float generation_factor_weekly = 1.0 + 0.1 * sin(2 * M_PI * timestep / (24.0 * 7.0) + M_PI / 4.0);                  // Weekly generation pattern
    float generation_factor_random = 1.0 + 0.05 * (float)rand() / RAND_MAX;                                             // Random generation variation
    float generation = base_generation * generation_factor_daily * generation_factor_weekly * generation_factor_random; // Calculate generation
    for (int i = 0; i < N_COMPONENTS; i++)                                                                              // Apply component-specific generation factors
    {
        generation += components[i].rating * 0.1 * sin(2 * M_PI * timestep / 24.0 + 2 * M_PI * i / N_COMPONENTS + M_PI / 4.0);
    }
    return generation;
}

// Function pointers for resilience strategies
void resilience_strategy_1(GridComponent *components)
{
    // Simulating by  activating a backup system (reducing imbalance by a percentage)
    printf("Activating backup system (reducing imbalance)\n");
    float imbalance_reduction = 0.2; // Reduce imbalance by 20%
    for (int i = 0; i < N_COMPONENTS; i++)
    {
        if (components[i].outage_duration[0] > 0)
        {
            components[i].outage_duration[0] -= 1;
            if (components[i].outage_duration[0] == 0)
            {
                components[i].component_state = "operational";
            }
        }
    }
    float imbalance_before = 0.0;
    for (int i = 0; i < N_COMPONENTS; i++)
    {
        imbalance_before += components[i].rating * (components[i].failure_rate * (components[i].component_state[0] == 'f' ? 1.0 : 0.0));
    }
    float imbalance_after = 0.0;
    for (int i = 0; i < N_COMPONENTS; i++)
    {
        imbalance_after += components[i].rating * (components[i].failure_rate * (components[i].component_state[0] == 'f' ? 1.0 : 0.0));
    }
    imbalance_after -= imbalance_before * imbalance_reduction;
    for (int i = 0; i < N_COMPONENTS; i++)
    {
        if (components[i].failure_rate > 0 && imbalance_after > 0)
        {
            components[i].outage_duration[0] += 1;
            components[i].component_state = "failed";
            imbalance_after -= components[i].rating * components[i].failure_rate;
        }
    }
}

void resilience_strategy_2(GridComponent *components)
{
    // Simulating load shedding (reducing demand to match supply)
    printf("Implementing load shedding (matching supply and demand)\n");
    float imbalance = 0.0;
    for (int i = 0; i < N_COMPONENTS; i++)
    {
        imbalance += components[i].rating * (components[i].failure_rate * (components[i].component_state[0] == 'f' ? 1.0 : 0.0));
    }
    if (imbalance > 0)
    {
        float demand_reduction = imbalance / (N_COMPONENTS * 0.9);
        for (int i = 0; i < N_COMPONENTS; i++)
        {
            if (components[i].failure_rate > 0)
            {
                components[i].rating *= 1.0 - demand_reduction / (components[i].failure_rate * 0.1);
            }
        }
    }
}

void resilience_strategy_3(GridComponent *components)
{
    // Simulating demand response (shifting demand to match supply)
    printf("Implementing demand response (shifting demand to match supply)\n");
    float imbalance = 0.0;
    for (int i = 0; i < N_COMPONENTS; i++)
    {
        imbalance += components[i].rating * (components[i].failure_rate * (components[i].component_state[0] == 'f' ? 1.0 : 0.0));
    }
    if (imbalance > 0)
    {
        float demand_shift = imbalance / (N_COMPONENTS * 0.9);
        for (int i = 0; i < N_COMPONENTS; i++)
        {
            if (components[i].failure_rate > 0)
            {
                components[i].rating *= 1.0 + demand_shift / (components[i].failure_rate * 0.1);
            }
        }
    }
}

// Function to calculate resilience metric
float calculate_resilience_metric(GridComponent *components)
{
    float energy_not_served = 0.0;
    for (int i = 0; i < N_COMPONENTS; i++)
    {
        energy_not_served += components[i].rating * components[i].outage_duration[0];
    }
    return energy_not_served;
}

int main()
{
    // Input Parameters
    int N; // Number of Monte Carlo iterations
    int T; // Number of simulation time steps

    // Defining Grid_Model as an array of GridComponent structures
    GridComponent Grid_Model[N_COMPONENTS];

    printf("Enter details for each component:\n");
    for (int i = 0; i < N_COMPONENTS; i++)
    {
        printf("Enter the values of failure rate and rating for component %d \n", i + 1);
        Grid_Model[i].component_state = "operational";
        printf("Enter failure rate: ");
        scanf("%f", &Grid_Model[i].failure_rate);
        printf("Enter rating: ");
        scanf("%f", &Grid_Model[i].rating);
        Grid_Model[i].outage_duration = (float *)calloc(1, sizeof(float));
    }

    // Defining System_Constraints as a dictionary structure

    struct SystemConstraints
    {
        float imbalance_threshold;
        float state_threshlod;
    };

    struct SystemConstraints System_Constraints;

    System_Constraints.imbalance_threshold = 0.05;

    float imbalance_threshold = System_Constraints.imbalance_threshold;

    // Declaration Section
    float delta_t;     // Time step duration
    float *P_fail;     // Pre-calculated failure probability for each component
    float *D;          // Demand level at each time step in each iteration
    float *G;          // Renewable energy generation at each time step in each iteration
    int **Oi;          // Outage state of each component at each time step in each iteration
    float **Imbalance; // Demand-supply imbalance at each time step in each iteration
    char ***Ci;        // State of each component at each time step in each iteration
    float *Rn;         // Resilience metric for each iteration

    // Initialization Section
    // Initialization for Grid_Model components
    delta_t = 1.0; // value for time step duration
    for (int i = 0; i < N_COMPONENTS; i++)
    {
        P_fail = (float *)malloc(sizeof(float) * sizeof(Grid_Model) / sizeof(Grid_Model[0]));
        P_fail[i] = 1 - exp(-Grid_Model[i].failure_rate * delta_t);
    }
    // Initialization for other variables
    D = (float *)calloc(N_ITERATIONS * N_TIMESTEPS, sizeof(float));
    G = (float *)calloc(N_ITERATIONS * N_TIMESTEPS, sizeof(float));
    Oi = (int **)calloc(N_ITERATIONS * N_TIMESTEPS, sizeof(int *) * N_COMPONENTS);
    for (int i = 0; i < N_ITERATIONS * N_TIMESTEPS; i++)
    {
        Oi[i] = (int *)calloc(N_COMPONENTS, sizeof(int));
    }
    Imbalance = (float **)calloc(N_ITERATIONS * N_TIMESTEPS, sizeof(float *));
    for (int i = 0; i < N_ITERATIONS * N_TIMESTEPS; i++)
    {
        Imbalance[i] = (float *)calloc(1, sizeof(float));
    }
    Ci = (char ***)calloc(N_ITERATIONS * N_TIMESTEPS, sizeof(char **) * N_COMPONENTS);
    for (int i = 0; i < N_ITERATIONS * N_TIMESTEPS; i++)
    {
        Ci[i] = (char **)calloc(N_COMPONENTS, sizeof(char *));
        for (int j = 0; j < N_COMPONENTS; j++)
        {
            Ci[i][j] = (char *)calloc(16, sizeof(char));
            strcpy(Ci[i][j], "operational");
        }
    }
    Rn = (float *)calloc(N_ITERATIONS, sizeof(float));

    

    // Initialization (t = 0)
    for (int n = 0; n < N_ITERATIONS; n++)
    {
        for (int t = 0; t < N_TIMESTEPS; t++)
        {
            D[n * N_TIMESTEPS + t] = sample_demand(t, n, Grid_Model);
            G[n * N_TIMESTEPS + t] = sample_renewable_generation(t, Grid_Model);
        }
    }
    // State Update
    for (int n = 0; n < N_ITERATIONS; n++)
    {
        for (int t = 1; t < N_TIMESTEPS; t++)
        {
            for (int i = 0; i < N_COMPONENTS; i++)
            {
                if (strcmp(Ci[n * N_TIMESTEPS + t - 1][i], "operational") == 0 && ((float)rand() / RAND_MAX) < P_fail[i])
                {
                    strcpy(Ci[n * N_TIMESTEPS + t][i], "failed");
                    Oi[n * N_TIMESTEPS + t][i] = 1;
                    Grid_Model[i].outage_duration[0] = 1;
                }
                else
                {
                    strcpy(Ci[n * N_TIMESTEPS + t][i], Ci[n * N_TIMESTEPS + t - 1][i]);
                    Oi[n * N_TIMESTEPS + t][i] = 0;
                }
            }
        }
    }

    // Demand-Supply Check
    void (*Resilience_Strategies[])() = {&resilience_strategy_1, &resilience_strategy_2, &resilience_strategy_3};
    for (int n = 0; n < N_ITERATIONS; n++)
    {
        for (int t = 0; t < N_TIMESTEPS; t++)
        {
            Imbalance[n * N_TIMESTEPS + t][0] = D[n * N_TIMESTEPS + t] - G[n * N_TIMESTEPS + t];
            if (Imbalance[n * N_TIMESTEPS + t][0] > imbalance_threshold)
            {
                // Applying resilience strategy
                // Resilience_Strategies[1](Grid_Model);
                //Resilience_Strategies[0](Grid_Model);
                Resilience_Strategies[2](Grid_Model);
            }
        }
    }

    // Resilience Metric Calculation
    for (int n = 0; n < N_ITERATIONS; n++)
    {
        for (int i = 0; i < N_COMPONENTS; i++)
        {
            Rn[n] += calculate_resilience_metric(Grid_Model);
        }
    }

    // Result

    printf("Simulation Results:\n\n");
    printf("Resilience Metric: %f\n", Rn[0] / N_ITERATIONS);


    return 0;
}
