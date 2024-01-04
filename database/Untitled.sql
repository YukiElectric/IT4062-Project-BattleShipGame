CREATE TABLE `users` (
  `user_id` INT PRIMARY KEY NOT NULL AUTO_INCREMENT,
  `user_name` VARCHAR(255) NOT NULL,
  `email` VARCHAR(255) NOT NULL,
  `password` VARCHAR(255) NOT NULL,
  `win` INT DEFAULT 0,
  `loss` INT DEFAULT 0,
  `elo` INT DEFAULT 0,
  `online` INT DEFAULT 0,
  `is_active` INT DEFAULT 0,
  `created_at` DATETIME NOT NULL DEFAULT (CURRENT_TIMESTAMP),
  `updated_at` DATETIME NOT NULL DEFAULT (CURRENT_TIMESTAMP)
);

CREATE TABLE `battles` (
  `battle_id` INT PRIMARY KEY NOT NULL AUTO_INCREMENT,
  `started_at` DATETIME NOT NULL DEFAULT (CURRENT_TIMESTAMP),
  `ended_at` DATETIME DEFAULT (0);
  `player_one` INT NOT NULL,
  `player_two` INT NOT NULL,
  `result` INT NOT NULL DEFAULT 0
);

CREATE TABLE `ship_location` (
  `ship_location_id` INT PRIMARY KEY NOT NULL AUTO_INCREMENT,
  `battle_id` INT NOT NULL,
  `player_id` INT NOT NULL,
  `row` INT NOT NULL,
  `column` INT NOT NULL,
  `ship_order` INT NOT NULL
);

CREATE TABLE `shots` (
  `move_id` INT PRIMARY KEY NOT NULL AUTO_INCREMENT,
  `battle_id` INT NOT NULL,
  `row_one_shot` INT,
  `column_one_shot` INT,
  `row_two_shot` INT,
  `column_two_shot` INT,
  `shot_order` INT
);

ALTER TABLE `battles` ADD FOREIGN KEY (`player_one`) REFERENCES `users` (`user_id`);

ALTER TABLE `battles` ADD FOREIGN KEY (`player_two`) REFERENCES `users` (`user_id`);

ALTER TABLE `shots` ADD FOREIGN KEY (`battle_id`) REFERENCES `battles` (`battle_id`);

ALTER TABLE `ship_location` ADD FOREIGN KEY (`battle_id`) REFERENCES `battles` (`battle_id`);

ALTER TABLE `ship_location` ADD FOREIGN KEY (`player_id`) REFERENCES `users` (`user_id`);
