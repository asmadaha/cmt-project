#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* === PARAMETERS === */

#define N_AGENTS 100 /* Number of agents in the population */
#define N_STEPS 500 /* Number of simulation time steps */
#define DEGREE 4 /* Number of neighbors per agent (fixed degree network) */

/* Straregy identifiers */
#define ALLC 0 /* Always Cooperate */
#define ALLD 1 /* Always Defect */
#define TFT  2 /* Tit-for-Tat */
#define GRUD 3 /* Grudger */

/* Prisoner's Dilemma payoff matrix values */
#define R 3.0 /* Reward for mutual cooperation */
#define T 5.0 /* Temptation to defect */
#define S 0.0 /* Sucker's payoff */
#define P 1.0 /* Punishment for mutual defection */

/* === DATA STRUCTURES === */

/* Structure representing a single agent */
typedef struct {
    int strategy; /* Current strategy type */
    int last_action; /* Action taken in the previous interaction (1=C, 0=D) */
    int grudged; /*  */
    double payoff; /* Accumulated payoff during the current time step */
} Agent;

/* ===GLOBAL VARIABLES=== */

/* Array of agents */
Agent agents[N_AGENTS];
/* Fixed random network: each agent has degree neighbors */
int network[N_AGENTS][DEGREE];

/* === RANDOM === */

/* Generate a uniform random number in [0,1) */
double randu() {
    return rand() / (double) RAND_MAX;
}

/* ===================== NETWORK ===================== */

/* Initialize a fixed random interaction network */
void init_network() {
    for (int i = 0; i < N_AGENTS; i++) {
        for (int d = 0; d < DEGREE; d++) {
            /* Randomly assign neighbors (self-links allowed for simplicity */
            network[i][d] = rand() % N_AGENTS;
        }
    }
}

/* ===================== AGENTS ===================== */

/* Initialize agent strategies and internal state */
void init_agents() {
    for (int i = 0; i < N_AGENTS; i++) {
        agents[i].strategy = rand() % 4; /* Random initial strategy */
        agents[i].last_action = 1; /* Start by cooperating */
        agents[i].grudged = 0; /* No grudges initially */
        agents[i].payoff = 0.0; /* Zero initial payoff */
    }
}

/* ===================== ACTION ===================== */

/* Determine agent i's action when interacting with agent j*/
int choose_action(int i, int j) {
    Agent *a = &agents[i];
    Agent *b = &agents[j];

    if (a->strategy == ALLC) return 1;
    if (a->strategy == ALLD) return 0;
    if (a->strategy == TFT) return b->last_action;
    if (a->strategy == GRUD) return a->grudged ? 0 : 1;

    /* Default fallback (should not occur) */
    return 1;
}

/* ===================== PAYOFF CALCULATION ===================== */

/* Apply Prisoner's Dilemma payoffs based on actions */
void apply_payoff(int i, int j, int ai, int aj) {
    if (ai == 1 && aj == 1) {
        agents[i].payoff += R;
        agents[j].payoff += R;
    } else if (ai == 1 && aj == 0) {
        agents[i].payoff += S;
        agents[j].payoff += T;
    } else if (ai == 0 && aj == 1) {
        agents[i].payoff += T;
        agents[j].payoff += S;
    } else {
        agents[i].payoff += P;
        agents[j].payoff += P;
    }
}

/* ===================== STRATEGY UPDATE: IMITATION DYNAMICS ===================== */

/* Perform a payoff-based imitation step using a Fermi update rule. 
Agents probabilistically imitate more successful neighbors. */
void imitation_step(double beta) {
    for (int i = 0; i < N_AGENTS; i++) {
        int j = network[i][rand() % DEGREE];
        double pi = agents[i].payoff;
        double pj = agents[j].payoff;

        /* Fermi imitation probability */
        double prob = 1.0 / (1.0 + exp(-beta * (pj - pi)));
        if (randu() < prob) {
            agents[i].strategy = agents[j].strategy;
        }
    }
}

/* ===================== MAIN ===================== */

int main(int argc, char *argv[]) {

    /* Check command-line arguments */
    if (argc < 3) {
        printf("Usage: ./pd_simulation p_noise output.csv\n");
        return 1;
    }

    /* Parse noise probability */
    double p_noise = atof(argv[1]);
    
    /* Open output CSV file (which is input for MATLAB) */
    FILE *out = fopen(argv[2], "w");
    if (!out) {
        printf("Error opening output file\n");
        return 1;
    }

    /* Fixed seed for full reproducibility */
    srand(42);

    /* Initialize system */
    init_network();
    init_agents();

    /* Write CSV header */
    fprintf(out, "time,cooperation\n");

    /* Time evolution */
    for (int t = 0; t < N_STEPS; t++) {

        /* Reset payoffs */
        for (int i = 0; i < N_AGENTS; i++) {
            agents[i].payoff = 0.0;
        }

        int coop_count = 0;

        /* Pairwise interactions */
        for (int i = 0; i < N_AGENTS; i++) {
            for (int d = 0; d < DEGREE; d++) {
                int j = network[i][d];

                int ai = choose_action(i, j);
                int aj = choose_action(j, i);

                /* Apply stochastic noise */
                if (randu() < p_noise) ai = 1 - ai;
                if (randu() < p_noise) aj = 1 - aj;

                /* Update grudges */
                if (ai == 0) agents[j].grudged = 1;
                if (aj == 0) agents[i].grudged = 1;

                /* Apply payoffs */
                apply_payoff(i, j, ai, aj);

                /* Store last actions */
                agents[i].last_action = ai;
                agents[j].last_action = aj;

                if (ai == 1) coop_count++;
            }
        }

       /* Strategy update */
        imitation_step(1.0);

        /* Record cooperation fraction */
        double coop_frac = coop_count / (double)(N_AGENTS * DEGREE);
        fprintf(out, "%d,%f\n", t, coop_frac);
    }

    /* Compute final strategy composition */
    int count[4] = {0, 0, 0, 0};
    for (int i = 0; i < N_AGENTS; i++) {
        count[agents[i].strategy]++;
    }

    /* Append final strategy fractions */
    fprintf(out, "#FINAL_STRATEGIES\n");
    fprintf(out, "%f,%f,%f,%f\n",
        count[ALLC] / (double)N_AGENTS,
        count[ALLD] / (double)N_AGENTS,
        count[TFT]  / (double)N_AGENTS,
        count[GRUD] / (double)N_AGENTS
    );

    fclose(out);
    return 0;
}

