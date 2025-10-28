#pragma once
typedef struct {
    int max_health_with_perk; // 7
    int max_health; // 6

    int turret_ammo; // 128
    int turret_shots; // 2
    int turret_reload; // 10
    int turret_move; // 10
    int turret_health; // 20

    int kill_ammo_bonus; // 7
    int ammo_cap; // 15

    int kill_health_bonus; // 1
    int kill_health_cap; // 3

    int instant_heal_potion_drop_health_threshold; // 1
    int potion_turbo_mode_effect_amount; // 2

    int initial_potion_healing_counter; // 25
    int initial_potion_shield_counter; // 15

    int potion_duration_big_boost; // 250
    int potion_duration_mini_boost; // 50

    int multiplied_gold_mode_initial_gold; // 20
    int multiplied_gold_mode_cluster_strength; // 5
    int cluster_strength; // 16

    int min_starting_ammo; // 10
    int min_starting_health; // 3
    int initial_gold_cap; // 5

    int healing_powerup_amount; // 3
    int healing_powerup_amount_brutal; // 2
    int healing_powerup_perk_bonus; // 1
    int healing_powerup_multiplier_overpowered; // 3
    int healing_powerup_cost; // 1
    int protection_powerup_cost; // 2
    int protection_powerup_amount; // 1
    int protection_powerup_perk_bonus; // 2
    int turret_powerup_cost; // 3
    int turret_powerup_cost_brutal; // 4
    int turret_powerup_perk_blast_intensity; // 1
    int turret_powerup_perk_blast_directions; // 32
    double turret_powerup_overpowered_speed_boost; // 2
    int blast_powerup_cost; // 6
    int blast_powerup_cost_brutal; // 8
    double blast_powerup_projectile_speed; // 0.5
    int blast_powerup_overpowered_blast_intensity; // 16
    int blast_powerup_overpowered_blast_directions; // 16

    int overpriced_powerups_cost_increase; // 2
    int boss_killing_xp_bonus; // 500

    int brutal_enemy_health_bonus; // 1

    int fast_potion_reload_divider; // 2
    int doubled_shots_shot_multiplier; // 2
    int boost_potion_shot_multiplier; // 2
    double bullet_spread_multiplier; // 1

    int breakable_wall_durability; // 5

    int weapon_1_num_shots; // 1
    int weapon_1_rate; // 7
    int weapon_1_brutal_shots; // 1
    int weapon_1_brutal_rate; // 12

    int weapon_2_num_shots; // 6
    int weapon_2_rate; // 200
    int weapon_2_brutal_shots; // 4
    int weapon_2_brutal_rate; // 30

    int undetected_enemy_move_probability; // 30

    int enemy_speed; // 1
    int fast_enemy_speed; // 2

    int plr_speed; // 4
    int fast_potion_plr_speed_bonus; // 1
    int fast_potion_plr_speed_bonus_turbo; // 2

    int perk_xp_base; // 300
    double perk_xp_level_multiplier; // 2
} GameTuningParams;

const GameTuningParams *get_tuning_params();
