-- Исправляем структуру таблицы counterparties
CREATE TABLE IF NOT EXISTS counterparties_fixed (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    inn TEXT UNIQUE,
    kpp TEXT DEFAULT '',
    address TEXT DEFAULT '',
    phone TEXT DEFAULT '',
    email TEXT DEFAULT '',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Копируем данные из старой таблицы
INSERT OR IGNORE INTO counterparties_fixed (id, name, inn, kpp, address, phone, email, created_at)
SELECT id, name, inn, 
       COALESCE(kpp, ''),
       COALESCE(address, ''),
       COALESCE(phone, ''),
       COALESCE(email, ''),
       COALESCE(created_at, CURRENT_TIMESTAMP)
FROM counterparties;

-- Удаляем старую таблицу и переименовываем новую
DROP TABLE counterparties;
ALTER TABLE counterparties_fixed RENAME TO counterparties;
