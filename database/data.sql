CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_name VARCHAR(100) UNIQUE NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    password VARCHAR(100) NOT NULL,
    win INTEGER DEFAULT 0,
    lose INTEGER DEFAULT 0,
    elo INTEGER DEFAULT 0,
    online INTEGER DEFAULT 1,
    is_active INTEGER DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TRIGGER update_timestamp
AFTER UPDATE ON users
FOR EACH ROW
BEGIN
    UPDATE users SET updated_at = CURRENT_TIMESTAMP WHERE id = NEW.id;
END;

CREATE TABLE battle (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    started_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    ended_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    player_one INT,
    player_two INT,
    result INTEGER DEFAULT 0,
    FOREIGN KEY(player_one) REFERENCES users(id),
    FOREIGN KEY(player_two) REFERENCES users(id)
);

CREATE TABLE ship_location (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    battle_id INT,
    player_id INT,
    row INTEGER,
    col INTEGER,
    ship_order INTEGER,
    FOREIGN KEY(battle_id) REFERENCES battle(id),
    FOREIGN KEY(player_id) REFERENCES users(id)
);

CREATE TABLE moves (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    battle_id INTEGER,
    row_one_move INTEGER,
    col_one_move INTEGER,
    row_two_move INTEGER,
    col_two_move INTEGER,
    move_order INTEGER,
    FOREIGN KEY(battle_id) REFERENCES battle(id)
);
