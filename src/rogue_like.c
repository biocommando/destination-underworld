#include "rogue_like.h"
#include <stddef.h>

static const int id_max_health_with_perk = 0;
static const int id_max_health = 1;
static const int id_turret_ammo = 2;
static const int id_turret_shots = 3;
static const int id_turret_reload = 4;
static const int id_turret_move = 5;
static const int id_turret_health = 6;
static const int id_kill_ammo_bonus = 7;
static const int id_ammo_cap = 8;
static const int id_kill_health_bonus = 9;
static const int id_kill_health_cap = 10;
static const int id_instant_heal_potion_drop_health_threshold = 11;
static const int id_potion_turbo_mode_effect_amount = 12;
static const int id_initial_potion_healing_counter = 13;
static const int id_initial_potion_shield_counter = 14;
static const int id_potion_duration_big_boost = 15;
static const int id_potion_duration_mini_boost = 16;
static const int id_multiplied_gold_mode_initial_gold = 17;
static const int id_multiplied_gold_mode_cluster_strength = 18;
static const int id_cluster_strength = 19;
static const int id_min_starting_ammo = 20;
static const int id_min_starting_health = 21;
static const int id_initial_gold_cap = 22;
static const int id_healing_powerup_amount = 23;
static const int id_healing_powerup_amount_brutal = 24;
static const int id_healing_powerup_perk_bonus = 25;
static const int id_healing_powerup_multiplier_overpowered = 26;
static const int id_healing_powerup_cost = 27;
static const int id_protection_powerup_cost = 28;
static const int id_protection_powerup_amount = 29;
static const int id_protection_powerup_perk_bonus = 30;
static const int id_turret_powerup_cost = 31;
static const int id_turret_powerup_cost_brutal = 32;
static const int id_turret_powerup_perk_blast_intensity = 33;
static const int id_turret_powerup_perk_blast_directions = 34;
static const int id_turret_powerup_overpowered_speed_boost = 35;
static const int id_blast_powerup_cost = 36;
static const int id_blast_powerup_cost_brutal = 37;
static const int id_blast_powerup_projectile_speed = 38;
static const int id_blast_powerup_overpowered_blast_intensity = 39;
static const int id_blast_powerup_overpowered_blast_directions = 40;
static const int id_blast_powerup_perk_timed_cluster_rate = 41;
static const int id_blast_powerup_perk_turret_enabled = 42;
static const int id_overpriced_powerups_cost_increase = 43;
static const int id_boss_killing_xp_bonus = 44;
static const int id_brutal_enemy_health_bonus = 45;
static const int id_fast_potion_reload_divider = 46;
static const int id_doubled_shots_shot_multiplier = 47;
static const int id_boost_potion_shot_multiplier = 48;
static const int id_bullet_spread_multiplier = 49;
static const int id_breakable_wall_durability = 50;
static const int id_weapon_1_num_shots = 51;
static const int id_weapon_1_rate = 52;
static const int id_weapon_1_brutal_shots = 53;
static const int id_weapon_1_brutal_rate = 54;
static const int id_weapon_2_num_shots = 55;
static const int id_weapon_2_rate = 56;
static const int id_weapon_2_brutal_shots = 57;
static const int id_weapon_2_brutal_rate = 58;
static const int id_undetected_enemy_move_probability = 59;
static const int id_enemy_speed = 60;
static const int id_fast_enemy_speed = 61;
static const int id_plr_speed = 62;
static const int id_fast_potion_plr_speed_bonus = 63;
static const int id_fast_potion_plr_speed_bonus_turbo = 64;
static const int id_perk_xp_base = 65;
static const int id_perk_xp_level_multiplier = 66;
static const int id_blood_stains_enabled = 67;

double get_tuning_param_current_value(const GameTuningParams *params, int id)
{
    if (id == id_max_health_with_perk)
        return params->max_health_with_perk;
    else if (id == id_max_health)
        return params->max_health;
    else if (id == id_turret_ammo)
        return params->turret_ammo;
    else if (id == id_turret_shots)
        return params->turret_shots;
    else if (id == id_turret_reload)
        return params->turret_reload;
    else if (id == id_turret_move)
        return params->turret_move;
    else if (id == id_turret_health)
        return params->turret_health;
    else if (id == id_kill_ammo_bonus)
        return params->kill_ammo_bonus;
    else if (id == id_ammo_cap)
        return params->ammo_cap;
    else if (id == id_kill_health_bonus)
        return params->kill_health_bonus;
    else if (id == id_kill_health_cap)
        return params->kill_health_cap;
    else if (id == id_instant_heal_potion_drop_health_threshold)
        return params->instant_heal_potion_drop_health_threshold;
    else if (id == id_potion_turbo_mode_effect_amount)
        return params->potion_turbo_mode_effect_amount;
    else if (id == id_initial_potion_healing_counter)
        return params->initial_potion_healing_counter;
    else if (id == id_initial_potion_shield_counter)
        return params->initial_potion_shield_counter;
    else if (id == id_potion_duration_big_boost)
        return params->potion_duration_big_boost;
    else if (id == id_potion_duration_mini_boost)
        return params->potion_duration_mini_boost;
    else if (id == id_multiplied_gold_mode_initial_gold)
        return params->multiplied_gold_mode_initial_gold;
    else if (id == id_multiplied_gold_mode_cluster_strength)
        return params->multiplied_gold_mode_cluster_strength;
    else if (id == id_cluster_strength)
        return params->cluster_strength;
    else if (id == id_min_starting_ammo)
        return params->min_starting_ammo;
    else if (id == id_min_starting_health)
        return params->min_starting_health;
    else if (id == id_initial_gold_cap)
        return params->initial_gold_cap;
    else if (id == id_healing_powerup_amount)
        return params->healing_powerup_amount;
    else if (id == id_healing_powerup_amount_brutal)
        return params->healing_powerup_amount_brutal;
    else if (id == id_healing_powerup_perk_bonus)
        return params->healing_powerup_perk_bonus;
    else if (id == id_healing_powerup_multiplier_overpowered)
        return params->healing_powerup_multiplier_overpowered;
    else if (id == id_healing_powerup_cost)
        return params->healing_powerup_cost;
    else if (id == id_protection_powerup_cost)
        return params->protection_powerup_cost;
    else if (id == id_protection_powerup_amount)
        return params->protection_powerup_amount;
    else if (id == id_protection_powerup_perk_bonus)
        return params->protection_powerup_perk_bonus;
    else if (id == id_turret_powerup_cost)
        return params->turret_powerup_cost;
    else if (id == id_turret_powerup_cost_brutal)
        return params->turret_powerup_cost_brutal;
    else if (id == id_turret_powerup_perk_blast_intensity)
        return params->turret_powerup_perk_blast_intensity;
    else if (id == id_turret_powerup_perk_blast_directions)
        return params->turret_powerup_perk_blast_directions;
    else if (id == id_turret_powerup_overpowered_speed_boost)
        return params->turret_powerup_overpowered_speed_boost;
    else if (id == id_blast_powerup_cost)
        return params->blast_powerup_cost;
    else if (id == id_blast_powerup_cost_brutal)
        return params->blast_powerup_cost_brutal;
    else if (id == id_blast_powerup_projectile_speed)
        return params->blast_powerup_projectile_speed;
    else if (id == id_blast_powerup_overpowered_blast_intensity)
        return params->blast_powerup_overpowered_blast_intensity;
    else if (id == id_blast_powerup_overpowered_blast_directions)
        return params->blast_powerup_overpowered_blast_directions;
    else if (id == id_blast_powerup_perk_timed_cluster_rate)
        return params->blast_powerup_perk_timed_cluster_rate;
    else if (id == id_blast_powerup_perk_turret_enabled)
        return params->blast_powerup_perk_turret_enabled;
    else if (id == id_overpriced_powerups_cost_increase)
        return params->overpriced_powerups_cost_increase;
    else if (id == id_boss_killing_xp_bonus)
        return params->boss_killing_xp_bonus;
    else if (id == id_brutal_enemy_health_bonus)
        return params->brutal_enemy_health_bonus;
    else if (id == id_fast_potion_reload_divider)
        return params->fast_potion_reload_divider;
    else if (id == id_doubled_shots_shot_multiplier)
        return params->doubled_shots_shot_multiplier;
    else if (id == id_boost_potion_shot_multiplier)
        return params->boost_potion_shot_multiplier;
    else if (id == id_bullet_spread_multiplier)
        return params->bullet_spread_multiplier;
    else if (id == id_breakable_wall_durability)
        return params->breakable_wall_durability;
    else if (id == id_weapon_1_num_shots)
        return params->weapon_1_num_shots;
    else if (id == id_weapon_1_rate)
        return params->weapon_1_rate;
    else if (id == id_weapon_1_brutal_shots)
        return params->weapon_1_brutal_shots;
    else if (id == id_weapon_1_brutal_rate)
        return params->weapon_1_brutal_rate;
    else if (id == id_weapon_2_num_shots)
        return params->weapon_2_num_shots;
    else if (id == id_weapon_2_rate)
        return params->weapon_2_rate;
    else if (id == id_weapon_2_brutal_shots)
        return params->weapon_2_brutal_shots;
    else if (id == id_weapon_2_brutal_rate)
        return params->weapon_2_brutal_rate;
    else if (id == id_undetected_enemy_move_probability)
        return params->undetected_enemy_move_probability;
    else if (id == id_enemy_speed)
        return params->enemy_speed;
    else if (id == id_fast_enemy_speed)
        return params->fast_enemy_speed;
    else if (id == id_plr_speed)
        return params->plr_speed;
    else if (id == id_fast_potion_plr_speed_bonus)
        return params->fast_potion_plr_speed_bonus;
    else if (id == id_fast_potion_plr_speed_bonus_turbo)
        return params->fast_potion_plr_speed_bonus_turbo;
    else if (id == id_perk_xp_base)
        return params->perk_xp_base;
    else if (id == id_perk_xp_level_multiplier)
        return params->perk_xp_level_multiplier;
    else if (id == id_blood_stains_enabled)
        return params->blood_stains_enabled;
    return 0;
}

void modify_tuning_params(GameTuningParams *params, const GameTuningModifier *modifier)
{
    double amount = modifier->amount;
    int id = modifier->param_id;
    double current_val = get_tuning_param_current_value(params, id);
    double new_val = current_val + modifier->amount;
    if ((current_val > 0 && new_val < 0.01) || (current_val <= 0 && new_val > -0.01))
        return;
    if (id == id_max_health_with_perk)
        params->max_health_with_perk += amount;
    else if (id == id_max_health)
        params->max_health += amount;
    else if (id == id_turret_ammo)
        params->turret_ammo += amount;
    else if (id == id_turret_shots)
        params->turret_shots += amount;
    else if (id == id_turret_reload)
        params->turret_reload += amount;
    else if (id == id_turret_move)
        params->turret_move += amount;
    else if (id == id_turret_health)
        params->turret_health += amount;
    else if (id == id_kill_ammo_bonus)
        params->kill_ammo_bonus += amount;
    else if (id == id_ammo_cap)
        params->ammo_cap += amount;
    else if (id == id_kill_health_bonus)
        params->kill_health_bonus += amount;
    else if (id == id_kill_health_cap)
        params->kill_health_cap += amount;
    else if (id == id_instant_heal_potion_drop_health_threshold)
        params->instant_heal_potion_drop_health_threshold += amount;
    else if (id == id_potion_turbo_mode_effect_amount)
        params->potion_turbo_mode_effect_amount += amount;
    else if (id == id_initial_potion_healing_counter)
        params->initial_potion_healing_counter += amount;
    else if (id == id_initial_potion_shield_counter)
        params->initial_potion_shield_counter += amount;
    else if (id == id_potion_duration_big_boost)
        params->potion_duration_big_boost += amount;
    else if (id == id_potion_duration_mini_boost)
        params->potion_duration_mini_boost += amount;
    else if (id == id_multiplied_gold_mode_initial_gold)
        params->multiplied_gold_mode_initial_gold += amount;
    else if (id == id_multiplied_gold_mode_cluster_strength)
        params->multiplied_gold_mode_cluster_strength += amount;
    else if (id == id_cluster_strength)
        params->cluster_strength += amount;
    else if (id == id_min_starting_ammo)
        params->min_starting_ammo += amount;
    else if (id == id_min_starting_health)
        params->min_starting_health += amount;
    else if (id == id_initial_gold_cap)
        params->initial_gold_cap += amount;
    else if (id == id_healing_powerup_amount)
        params->healing_powerup_amount += amount;
    else if (id == id_healing_powerup_amount_brutal)
        params->healing_powerup_amount_brutal += amount;
    else if (id == id_healing_powerup_perk_bonus)
        params->healing_powerup_perk_bonus += amount;
    else if (id == id_healing_powerup_multiplier_overpowered)
        params->healing_powerup_multiplier_overpowered += amount;
    else if (id == id_healing_powerup_cost)
        params->healing_powerup_cost += amount;
    else if (id == id_protection_powerup_cost)
        params->protection_powerup_cost += amount;
    else if (id == id_protection_powerup_amount)
        params->protection_powerup_amount += amount;
    else if (id == id_protection_powerup_perk_bonus)
        params->protection_powerup_perk_bonus += amount;
    else if (id == id_turret_powerup_cost)
        params->turret_powerup_cost += amount;
    else if (id == id_turret_powerup_cost_brutal)
        params->turret_powerup_cost_brutal += amount;
    else if (id == id_turret_powerup_perk_blast_intensity)
        params->turret_powerup_perk_blast_intensity += amount;
    else if (id == id_turret_powerup_perk_blast_directions)
        params->turret_powerup_perk_blast_directions += amount;
    else if (id == id_turret_powerup_overpowered_speed_boost)
        params->turret_powerup_overpowered_speed_boost += amount;
    else if (id == id_blast_powerup_cost)
        params->blast_powerup_cost += amount;
    else if (id == id_blast_powerup_cost_brutal)
        params->blast_powerup_cost_brutal += amount;
    else if (id == id_blast_powerup_projectile_speed)
        params->blast_powerup_projectile_speed += amount;
    else if (id == id_blast_powerup_overpowered_blast_intensity)
        params->blast_powerup_overpowered_blast_intensity += amount;
    else if (id == id_blast_powerup_overpowered_blast_directions)
        params->blast_powerup_overpowered_blast_directions += amount;
    else if (id == id_blast_powerup_perk_timed_cluster_rate)
        params->blast_powerup_perk_timed_cluster_rate += amount;
    else if (id == id_blast_powerup_perk_turret_enabled)
        params->blast_powerup_perk_turret_enabled += amount;
    else if (id == id_overpriced_powerups_cost_increase)
        params->overpriced_powerups_cost_increase += amount;
    else if (id == id_boss_killing_xp_bonus)
        params->boss_killing_xp_bonus += amount;
    else if (id == id_brutal_enemy_health_bonus)
        params->brutal_enemy_health_bonus += amount;
    else if (id == id_fast_potion_reload_divider)
        params->fast_potion_reload_divider += amount;
    else if (id == id_doubled_shots_shot_multiplier)
        params->doubled_shots_shot_multiplier += amount;
    else if (id == id_boost_potion_shot_multiplier)
        params->boost_potion_shot_multiplier += amount;
    else if (id == id_bullet_spread_multiplier)
        params->bullet_spread_multiplier += amount;
    else if (id == id_breakable_wall_durability)
        params->breakable_wall_durability += amount;
    else if (id == id_weapon_1_num_shots)
        params->weapon_1_num_shots += amount;
    else if (id == id_weapon_1_rate)
        params->weapon_1_rate += amount;
    else if (id == id_weapon_1_brutal_shots)
        params->weapon_1_brutal_shots += amount;
    else if (id == id_weapon_1_brutal_rate)
        params->weapon_1_brutal_rate += amount;
    else if (id == id_weapon_2_num_shots)
        params->weapon_2_num_shots += amount;
    else if (id == id_weapon_2_rate)
        params->weapon_2_rate += amount;
    else if (id == id_weapon_2_brutal_shots)
        params->weapon_2_brutal_shots += amount;
    else if (id == id_weapon_2_brutal_rate)
        params->weapon_2_brutal_rate += amount;
    else if (id == id_undetected_enemy_move_probability)
        params->undetected_enemy_move_probability += amount;
    else if (id == id_enemy_speed)
        params->enemy_speed += amount;
    else if (id == id_fast_enemy_speed)
        params->fast_enemy_speed += amount;
    else if (id == id_plr_speed)
        params->plr_speed += amount;
    else if (id == id_fast_potion_plr_speed_bonus)
        params->fast_potion_plr_speed_bonus += amount;
    else if (id == id_fast_potion_plr_speed_bonus_turbo)
        params->fast_potion_plr_speed_bonus_turbo += amount;
    else if (id == id_perk_xp_base)
        params->perk_xp_base += amount;
    else if (id == id_perk_xp_level_multiplier)
        params->perk_xp_level_multiplier += amount;
    else if (id == id_blood_stains_enabled)
        params->blood_stains_enabled += amount;
}

static const char *name_max_health_with_perk = "max_health_with_perk";
static const char *name_max_health = "max_health";
static const char *name_turret_ammo = "turret_ammo";
static const char *name_turret_shots = "turret_shots";
static const char *name_turret_reload = "turret_reload";
static const char *name_turret_move = "turret_move";
static const char *name_turret_health = "turret_health";
static const char *name_kill_ammo_bonus = "kill_ammo_bonus";
static const char *name_ammo_cap = "ammo_cap";
static const char *name_kill_health_bonus = "kill_health_bonus";
static const char *name_kill_health_cap = "kill_health_cap";
static const char *name_instant_heal_potion_drop_health_threshold = "instant_heal_potion_drop_health_threshold";
static const char *name_potion_turbo_mode_effect_amount = "potion_turbo_mode_effect_amount";
static const char *name_initial_potion_healing_counter = "initial_potion_healing_counter";
static const char *name_initial_potion_shield_counter = "initial_potion_shield_counter";
static const char *name_potion_duration_big_boost = "potion_duration_big_boost";
static const char *name_potion_duration_mini_boost = "potion_duration_mini_boost";
static const char *name_multiplied_gold_mode_initial_gold = "multiplied_gold_mode_initial_gold";
static const char *name_multiplied_gold_mode_cluster_strength = "multiplied_gold_mode_cluster_strength";
static const char *name_cluster_strength = "cluster_strength";
static const char *name_min_starting_ammo = "min_starting_ammo";
static const char *name_min_starting_health = "min_starting_health";
static const char *name_initial_gold_cap = "initial_gold_cap";
static const char *name_healing_powerup_amount = "healing_powerup_amount";
static const char *name_healing_powerup_amount_brutal = "healing_powerup_amount_brutal";
static const char *name_healing_powerup_perk_bonus = "healing_powerup_perk_bonus";
static const char *name_healing_powerup_multiplier_overpowered = "healing_powerup_multiplier_overpowered";
static const char *name_healing_powerup_cost = "healing_powerup_cost";
static const char *name_protection_powerup_cost = "protection_powerup_cost";
static const char *name_protection_powerup_amount = "protection_powerup_amount";
static const char *name_protection_powerup_perk_bonus = "protection_powerup_perk_bonus";
static const char *name_turret_powerup_cost = "turret_powerup_cost";
static const char *name_turret_powerup_cost_brutal = "turret_powerup_cost_brutal";
static const char *name_turret_powerup_perk_blast_intensity = "turret_powerup_perk_blast_intensity";
static const char *name_turret_powerup_perk_blast_directions = "turret_powerup_perk_blast_directions";
static const char *name_turret_powerup_overpowered_speed_boost = "turret_powerup_overpowered_speed_boost";
static const char *name_blast_powerup_cost = "blast_powerup_cost";
static const char *name_blast_powerup_cost_brutal = "blast_powerup_cost_brutal";
static const char *name_blast_powerup_projectile_speed = "blast_powerup_projectile_speed";
static const char *name_blast_powerup_overpowered_blast_intensity = "blast_powerup_overpowered_blast_intensity";
static const char *name_blast_powerup_overpowered_blast_directions = "blast_powerup_overpowered_blast_directions";
static const char *name_blast_powerup_perk_timed_cluster_rate = "blast_powerup_perk_timed_cluster_rate";
static const char *name_blast_powerup_perk_turret_enabled = "blast_powerup_perk_turret_enabled";
static const char *name_overpriced_powerups_cost_increase = "overpriced_powerups_cost_increase";
static const char *name_boss_killing_xp_bonus = "boss_killing_xp_bonus";
static const char *name_brutal_enemy_health_bonus = "brutal_enemy_health_bonus";
static const char *name_fast_potion_reload_divider = "fast_potion_reload_divider";
static const char *name_doubled_shots_shot_multiplier = "doubled_shots_shot_multiplier";
static const char *name_boost_potion_shot_multiplier = "boost_potion_shot_multiplier";
static const char *name_bullet_spread_multiplier = "bullet_spread_multiplier";
static const char *name_breakable_wall_durability = "breakable_wall_durability";
static const char *name_weapon_1_num_shots = "weapon_1_num_shots";
static const char *name_weapon_1_rate = "weapon_1_rate";
static const char *name_weapon_1_brutal_shots = "weapon_1_brutal_shots";
static const char *name_weapon_1_brutal_rate = "weapon_1_brutal_rate";
static const char *name_weapon_2_num_shots = "weapon_2_num_shots";
static const char *name_weapon_2_rate = "weapon_2_rate";
static const char *name_weapon_2_brutal_shots = "weapon_2_brutal_shots";
static const char *name_weapon_2_brutal_rate = "weapon_2_brutal_rate";
static const char *name_undetected_enemy_move_probability = "undetected_enemy_move_probability";
static const char *name_enemy_speed = "enemy_speed";
static const char *name_fast_enemy_speed = "fast_enemy_speed";
static const char *name_plr_speed = "plr_speed";
static const char *name_fast_potion_plr_speed_bonus = "fast_potion_plr_speed_bonus";
static const char *name_fast_potion_plr_speed_bonus_turbo = "fast_potion_plr_speed_bonus_turbo";
static const char *name_perk_xp_base = "perk_xp_base";
static const char *name_perk_xp_level_multiplier = "perk_xp_level_multiplier";
static const char *name_blood_stains_enabled = "blood_stains_enabled";

const char *get_tuning_param_description(int param_id)
{
    if (param_id == id_max_health_with_perk)
        return name_max_health_with_perk;
    if (param_id == id_max_health)
        return name_max_health;
    if (param_id == id_turret_ammo)
        return name_turret_ammo;
    if (param_id == id_turret_shots)
        return name_turret_shots;
    if (param_id == id_turret_reload)
        return name_turret_reload;
    if (param_id == id_turret_move)
        return name_turret_move;
    if (param_id == id_turret_health)
        return name_turret_health;
    if (param_id == id_kill_ammo_bonus)
        return name_kill_ammo_bonus;
    if (param_id == id_ammo_cap)
        return name_ammo_cap;
    if (param_id == id_kill_health_bonus)
        return name_kill_health_bonus;
    if (param_id == id_kill_health_cap)
        return name_kill_health_cap;
    if (param_id == id_instant_heal_potion_drop_health_threshold)
        return name_instant_heal_potion_drop_health_threshold;
    if (param_id == id_potion_turbo_mode_effect_amount)
        return name_potion_turbo_mode_effect_amount;
    if (param_id == id_initial_potion_healing_counter)
        return name_initial_potion_healing_counter;
    if (param_id == id_initial_potion_shield_counter)
        return name_initial_potion_shield_counter;
    if (param_id == id_potion_duration_big_boost)
        return name_potion_duration_big_boost;
    if (param_id == id_potion_duration_mini_boost)
        return name_potion_duration_mini_boost;
    if (param_id == id_multiplied_gold_mode_initial_gold)
        return name_multiplied_gold_mode_initial_gold;
    if (param_id == id_multiplied_gold_mode_cluster_strength)
        return name_multiplied_gold_mode_cluster_strength;
    if (param_id == id_cluster_strength)
        return name_cluster_strength;
    if (param_id == id_min_starting_ammo)
        return name_min_starting_ammo;
    if (param_id == id_min_starting_health)
        return name_min_starting_health;
    if (param_id == id_initial_gold_cap)
        return name_initial_gold_cap;
    if (param_id == id_healing_powerup_amount)
        return name_healing_powerup_amount;
    if (param_id == id_healing_powerup_amount_brutal)
        return name_healing_powerup_amount_brutal;
    if (param_id == id_healing_powerup_perk_bonus)
        return name_healing_powerup_perk_bonus;
    if (param_id == id_healing_powerup_multiplier_overpowered)
        return name_healing_powerup_multiplier_overpowered;
    if (param_id == id_healing_powerup_cost)
        return name_healing_powerup_cost;
    if (param_id == id_protection_powerup_cost)
        return name_protection_powerup_cost;
    if (param_id == id_protection_powerup_amount)
        return name_protection_powerup_amount;
    if (param_id == id_protection_powerup_perk_bonus)
        return name_protection_powerup_perk_bonus;
    if (param_id == id_turret_powerup_cost)
        return name_turret_powerup_cost;
    if (param_id == id_turret_powerup_cost_brutal)
        return name_turret_powerup_cost_brutal;
    if (param_id == id_turret_powerup_perk_blast_intensity)
        return name_turret_powerup_perk_blast_intensity;
    if (param_id == id_turret_powerup_perk_blast_directions)
        return name_turret_powerup_perk_blast_directions;
    if (param_id == id_turret_powerup_overpowered_speed_boost)
        return name_turret_powerup_overpowered_speed_boost;
    if (param_id == id_blast_powerup_cost)
        return name_blast_powerup_cost;
    if (param_id == id_blast_powerup_cost_brutal)
        return name_blast_powerup_cost_brutal;
    if (param_id == id_blast_powerup_projectile_speed)
        return name_blast_powerup_projectile_speed;
    if (param_id == id_blast_powerup_overpowered_blast_intensity)
        return name_blast_powerup_overpowered_blast_intensity;
    if (param_id == id_blast_powerup_overpowered_blast_directions)
        return name_blast_powerup_overpowered_blast_directions;
    if (param_id == id_blast_powerup_perk_timed_cluster_rate)
        return name_blast_powerup_perk_timed_cluster_rate;
    if (param_id == id_blast_powerup_perk_turret_enabled)
        return name_blast_powerup_perk_turret_enabled;
    if (param_id == id_overpriced_powerups_cost_increase)
        return name_overpriced_powerups_cost_increase;
    if (param_id == id_boss_killing_xp_bonus)
        return name_boss_killing_xp_bonus;
    if (param_id == id_brutal_enemy_health_bonus)
        return name_brutal_enemy_health_bonus;
    if (param_id == id_fast_potion_reload_divider)
        return name_fast_potion_reload_divider;
    if (param_id == id_doubled_shots_shot_multiplier)
        return name_doubled_shots_shot_multiplier;
    if (param_id == id_boost_potion_shot_multiplier)
        return name_boost_potion_shot_multiplier;
    if (param_id == id_bullet_spread_multiplier)
        return name_bullet_spread_multiplier;
    if (param_id == id_breakable_wall_durability)
        return name_breakable_wall_durability;
    if (param_id == id_weapon_1_num_shots)
        return name_weapon_1_num_shots;
    if (param_id == id_weapon_1_rate)
        return name_weapon_1_rate;
    if (param_id == id_weapon_1_brutal_shots)
        return name_weapon_1_brutal_shots;
    if (param_id == id_weapon_1_brutal_rate)
        return name_weapon_1_brutal_rate;
    if (param_id == id_weapon_2_num_shots)
        return name_weapon_2_num_shots;
    if (param_id == id_weapon_2_rate)
        return name_weapon_2_rate;
    if (param_id == id_weapon_2_brutal_shots)
        return name_weapon_2_brutal_shots;
    if (param_id == id_weapon_2_brutal_rate)
        return name_weapon_2_brutal_rate;
    if (param_id == id_undetected_enemy_move_probability)
        return name_undetected_enemy_move_probability;
    if (param_id == id_enemy_speed)
        return name_enemy_speed;
    if (param_id == id_fast_enemy_speed)
        return name_fast_enemy_speed;
    if (param_id == id_plr_speed)
        return name_plr_speed;
    if (param_id == id_fast_potion_plr_speed_bonus)
        return name_fast_potion_plr_speed_bonus;
    if (param_id == id_fast_potion_plr_speed_bonus_turbo)
        return name_fast_potion_plr_speed_bonus_turbo;
    if (param_id == id_perk_xp_base)
        return name_perk_xp_base;
    if (param_id == id_perk_xp_level_multiplier)
        return name_perk_xp_level_multiplier;
    if (param_id == id_blood_stains_enabled)
        return name_blood_stains_enabled;
    return NULL;
}

static const int available_params_normal[] = {
    id_max_health_with_perk,
    id_max_health,
    //    id_turret_ammo,
    //    id_turret_shots,
    //    id_turret_reload,
    id_turret_move,
    //    id_turret_health,
    id_kill_ammo_bonus,
    id_ammo_cap,
    id_kill_health_bonus,
    id_kill_health_cap,
    //    id_instant_heal_potion_drop_health_threshold,
    //    id_potion_turbo_mode_effect_amount,
    id_initial_potion_healing_counter,
    id_initial_potion_shield_counter,
    id_potion_duration_big_boost,
    id_potion_duration_mini_boost,
    //    id_multiplied_gold_mode_initial_gold,
    //    id_multiplied_gold_mode_cluster_strength,
    id_cluster_strength,
    id_min_starting_ammo,
    id_min_starting_health,
    id_initial_gold_cap,
    id_healing_powerup_amount,
    //    id_healing_powerup_amount_brutal,
    id_healing_powerup_perk_bonus,
    //    id_healing_powerup_multiplier_overpowered,
    id_healing_powerup_cost,
    id_protection_powerup_cost,
    id_protection_powerup_amount,
    id_protection_powerup_perk_bonus,
    id_turret_powerup_cost,
    //    id_turret_powerup_cost_brutal,
    id_turret_powerup_perk_blast_intensity,
    id_turret_powerup_perk_blast_directions,
    //    id_turret_powerup_overpowered_speed_boost,
    id_blast_powerup_cost,
    //    id_blast_powerup_cost_brutal,
    id_blast_powerup_projectile_speed,
    //    id_blast_powerup_overpowered_blast_intensity,
    //    id_blast_powerup_overpowered_blast_directions,
    id_blast_powerup_perk_timed_cluster_rate,
    //    id_blast_powerup_perk_turret_enabled,
    //    id_overpriced_powerups_cost_increase,
    //    id_boss_killing_xp_bonus,
    //    id_brutal_enemy_health_bonus,
    //    id_fast_potion_reload_divider,
    //    id_doubled_shots_shot_multiplier,
    //    id_boost_potion_shot_multiplier,
    id_bullet_spread_multiplier,
    id_breakable_wall_durability,
    id_weapon_1_num_shots,
    id_weapon_1_rate,
    //    id_weapon_1_brutal_shots,
    //    id_weapon_1_brutal_rate,
    id_weapon_2_num_shots,
    id_weapon_2_rate,
    //    id_weapon_2_brutal_shots,
    //    id_weapon_2_brutal_rate,
    id_undetected_enemy_move_probability,
    id_enemy_speed,
    id_fast_enemy_speed,
    id_plr_speed,
    id_fast_potion_plr_speed_bonus,
    //    id_fast_potion_plr_speed_bonus_turbo,
    //    id_perk_xp_base,
    //    id_perk_xp_level_multiplier,
    //    id_blood_stains_enabled,
};

static const int available_params_brutal[] = {
    id_max_health_with_perk,
    id_max_health,
    //    id_turret_ammo,
    //    id_turret_shots,
    //    id_turret_reload,
    id_turret_move,
    //    id_turret_health,
    id_kill_ammo_bonus,
    id_ammo_cap,
    id_kill_health_bonus,
    id_kill_health_cap,
    //    id_instant_heal_potion_drop_health_threshold,
    //    id_potion_turbo_mode_effect_amount,
    id_initial_potion_healing_counter,
    id_initial_potion_shield_counter,
    id_potion_duration_big_boost,
    id_potion_duration_mini_boost,
    //    id_multiplied_gold_mode_initial_gold,
    //    id_multiplied_gold_mode_cluster_strength,
    id_cluster_strength,
    id_min_starting_ammo,
    id_min_starting_health,
    id_initial_gold_cap,
    //    id_healing_powerup_amount,
    id_healing_powerup_amount_brutal,
    id_healing_powerup_perk_bonus,
    //    id_healing_powerup_multiplier_overpowered,
    id_healing_powerup_cost,
    id_protection_powerup_cost,
    id_protection_powerup_amount,
    id_protection_powerup_perk_bonus,
    //    id_turret_powerup_cost,
    id_turret_powerup_cost_brutal,
    id_turret_powerup_perk_blast_intensity,
    id_turret_powerup_perk_blast_directions,
    //    id_turret_powerup_overpowered_speed_boost,
    //    id_blast_powerup_cost,
    id_blast_powerup_cost_brutal,
    id_blast_powerup_projectile_speed,
    //    id_blast_powerup_overpowered_blast_intensity,
    //    id_blast_powerup_overpowered_blast_directions,
    id_blast_powerup_perk_timed_cluster_rate,
    //    id_blast_powerup_perk_turret_enabled,
    //    id_overpriced_powerups_cost_increase,
    //    id_boss_killing_xp_bonus,
    id_brutal_enemy_health_bonus,
    //    id_fast_potion_reload_divider,
    //    id_doubled_shots_shot_multiplier,
    //    id_boost_potion_shot_multiplier,
    id_bullet_spread_multiplier,
    id_breakable_wall_durability,
    //    id_weapon_1_num_shots,
    //    id_weapon_1_rate,
    id_weapon_1_brutal_shots,
    id_weapon_1_brutal_rate,
    //    id_weapon_2_num_shots,
    //    id_weapon_2_rate,
    id_weapon_2_brutal_shots,
    id_weapon_2_brutal_rate,
    id_undetected_enemy_move_probability,
    id_enemy_speed,
    id_fast_enemy_speed,
    id_plr_speed,
    id_fast_potion_plr_speed_bonus,
    //    id_fast_potion_plr_speed_bonus_turbo,
    //    id_perk_xp_base,
    //    id_perk_xp_level_multiplier,
    //    id_blood_stains_enabled,
};

static GameTuningModifier _get_tuning_param_modifier(int index, int is_bad, const int *available_params, size_t num_params)
{
    GameTuningModifier modifier;
    int id = available_params[index % num_params];
    modifier.param_id = id;
    if (id == id_max_health_with_perk)
    {
        modifier.amount = 1;
    }
    else if (id == id_max_health)
    {
        modifier.amount = 1;
    }
    else if (id == id_turret_ammo)
    {
        modifier.amount = 1;
    }
    else if (id == id_turret_shots)
    {
        modifier.amount = 1;
    }
    else if (id == id_turret_reload)
    {
        modifier.amount = 1;
    }
    else if (id == id_turret_move)
    {
        modifier.amount = 2;
    }
    else if (id == id_turret_health)
    {
        modifier.amount = 1;
    }
    else if (id == id_kill_ammo_bonus)
    {
        modifier.amount = 1;
    }
    else if (id == id_ammo_cap)
    {
        modifier.amount = 3;
    }
    else if (id == id_kill_health_bonus)
    {
        modifier.amount = 1;
    }
    else if (id == id_kill_health_cap)
    {
        modifier.amount = 1;
    }
    else if (id == id_instant_heal_potion_drop_health_threshold)
    {
        modifier.amount = 1;
    }
    else if (id == id_potion_turbo_mode_effect_amount)
    {
        modifier.amount = 1;
    }
    else if (id == id_initial_potion_healing_counter)
    {
        modifier.amount = 10;
    }
    else if (id == id_initial_potion_shield_counter)
    {
        modifier.amount = 10;
    }
    else if (id == id_potion_duration_big_boost)
    {
        modifier.amount = 5;
    }
    else if (id == id_potion_duration_mini_boost)
    {
        modifier.amount = 5;
    }
    else if (id == id_multiplied_gold_mode_initial_gold)
    {
        modifier.amount = 1;
    }
    else if (id == id_multiplied_gold_mode_cluster_strength)
    {
        modifier.amount = 1;
    }
    else if (id == id_cluster_strength)
    {
        modifier.amount = 4;
    }
    else if (id == id_min_starting_ammo)
    {
        modifier.amount = 3;
    }
    else if (id == id_min_starting_health)
    {
        modifier.amount = 1;
    }
    else if (id == id_initial_gold_cap)
    {
        modifier.amount = 1;
    }
    else if (id == id_healing_powerup_amount)
    {
        modifier.amount = 1;
    }
    else if (id == id_healing_powerup_amount_brutal)
    {
        modifier.amount = 1;
    }
    else if (id == id_healing_powerup_perk_bonus)
    {
        modifier.amount = 1;
    }
    else if (id == id_healing_powerup_multiplier_overpowered)
    {
        modifier.amount = 1;
    }
    else if (id == id_healing_powerup_cost)
    {
        modifier.amount = -1;
    }
    else if (id == id_protection_powerup_cost)
    {
        modifier.amount = -1;
    }
    else if (id == id_protection_powerup_amount)
    {
        modifier.amount = 1;
    }
    else if (id == id_protection_powerup_perk_bonus)
    {
        modifier.amount = 1;
    }
    else if (id == id_turret_powerup_cost)
    {
        modifier.amount = -1;
    }
    else if (id == id_turret_powerup_cost_brutal)
    {
        modifier.amount = -1;
    }
    else if (id == id_turret_powerup_perk_blast_intensity)
    {
        modifier.amount = 1;
    }
    else if (id == id_turret_powerup_perk_blast_directions)
    {
        modifier.amount = 4;
    }
    else if (id == id_turret_powerup_overpowered_speed_boost)
    {
        modifier.amount = 1;
    }
    else if (id == id_blast_powerup_cost)
    {
        modifier.amount = -1;
    }
    else if (id == id_blast_powerup_cost_brutal)
    {
        modifier.amount = -1;
    }
    else if (id == id_blast_powerup_projectile_speed)
    {
        modifier.amount = 1;
    }
    else if (id == id_blast_powerup_overpowered_blast_intensity)
    {
        modifier.amount = 3;
    }
    else if (id == id_blast_powerup_overpowered_blast_directions)
    {
        modifier.amount = 4;
    }
    else if (id == id_blast_powerup_perk_timed_cluster_rate)
    {
        modifier.amount = -2;
    }
    else if (id == id_blast_powerup_perk_turret_enabled)
    {
        modifier.amount = 1;
    }
    else if (id == id_overpriced_powerups_cost_increase)
    {
        modifier.amount = 1;
    }
    else if (id == id_boss_killing_xp_bonus)
    {
        modifier.amount = 1;
    }
    else if (id == id_brutal_enemy_health_bonus)
    {
        modifier.amount = -1;
    }
    else if (id == id_fast_potion_reload_divider)
    {
        modifier.amount = 1;
    }
    else if (id == id_doubled_shots_shot_multiplier)
    {
        modifier.amount = 1;
    }
    else if (id == id_boost_potion_shot_multiplier)
    {
        modifier.amount = 1;
    }
    else if (id == id_bullet_spread_multiplier)
    {
        modifier.amount = -2;
    }
    else if (id == id_breakable_wall_durability)
    {
        modifier.amount = -5;
    }
    else if (id == id_weapon_1_num_shots)
    {
        modifier.amount = 1;
    }
    else if (id == id_weapon_1_rate)
    {
        modifier.amount = -2;
    }
    else if (id == id_weapon_1_brutal_shots)
    {
        modifier.amount = 1;
    }
    else if (id == id_weapon_1_brutal_rate)
    {
        modifier.amount = -2;
    }
    else if (id == id_weapon_2_num_shots)
    {
        modifier.amount = 1;
    }
    else if (id == id_weapon_2_rate)
    {
        modifier.amount = -5;
    }
    else if (id == id_weapon_2_brutal_shots)
    {
        modifier.amount = 1;
    }
    else if (id == id_weapon_2_brutal_rate)
    {
        modifier.amount = -2;
    }
    else if (id == id_undetected_enemy_move_probability)
    {
        modifier.amount = -20;
    }
    else if (id == id_enemy_speed)
    {
        modifier.amount = -1;
    }
    else if (id == id_fast_enemy_speed)
    {
        modifier.amount = -1;
    }
    else if (id == id_plr_speed)
    {
        modifier.amount = 1;
    }
    else if (id == id_fast_potion_plr_speed_bonus)
    {
        modifier.amount = 1;
    }
    else if (id == id_fast_potion_plr_speed_bonus_turbo)
    {
        modifier.amount = 1;
    }
    else if (id == id_perk_xp_base)
    {
        modifier.amount = 100;
    }
    else if (id == id_perk_xp_level_multiplier)
    {
        modifier.amount = 1;
    }
    if (is_bad)
        modifier.amount *= -1;
    return modifier;
}

GameTuningModifier get_tuning_param_modifier(int index, int is_bad)
{
    return _get_tuning_param_modifier(index, is_bad, available_params_normal, sizeof(available_params_normal) / sizeof(int));
}

GameTuningModifier get_tuning_param_modifier_brutal(int index, int is_bad)
{
    return _get_tuning_param_modifier(index, is_bad, available_params_brutal, sizeof(available_params_brutal) / sizeof(int));
}
