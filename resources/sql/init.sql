-- Таблица плана счетов РСБУ
CREATE TABLE IF NOT EXISTS accounts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    code TEXT NOT NULL UNIQUE,          -- Код счета (например, "50", "51.01")
    name TEXT NOT NULL,                 -- Наименование счета
    type INTEGER NOT NULL,              -- 0=Активный, 1=Пассивный, 2=Активно-пассивный
    parent_id INTEGER,                  -- Родительский счет для иерархии
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    FOREIGN KEY (parent_id) REFERENCES accounts(id) ON DELETE CASCADE
);

-- Таблица контрагентов
CREATE TABLE IF NOT EXISTS counterparties (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,                 -- Наименование контрагента
    inn TEXT UNIQUE,                    -- ИНН
    kpp TEXT,                           -- КПП
    address TEXT,                       -- Адрес
    phone TEXT,                         -- Телефон
    email TEXT,                         -- Email
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Таблица проводок
CREATE TABLE IF NOT EXISTS transactions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    transaction_date DATE NOT NULL,     -- Дата проводки
    debit_account_id INTEGER NOT NULL,  -- Счет дебета
    credit_account_id INTEGER NOT NULL, -- Счет кредита
    amount DECIMAL(15,2) NOT NULL,     -- Сумма
    description TEXT,                   -- Описание
    document_number TEXT,               -- Номер документа
    document_date DATE,                 -- Дата документа
    counterparty_id INTEGER,            -- Контрагент
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    FOREIGN KEY (debit_account_id) REFERENCES accounts(id),
    FOREIGN KEY (credit_account_id) REFERENCES accounts(id),
    FOREIGN KEY (counterparty_id) REFERENCES counterparties(id),
    
    CHECK (amount > 0)
);

-- Индексы для ускорения поиска
CREATE INDEX IF NOT EXISTS idx_transactions_date ON transactions(transaction_date);
CREATE INDEX IF NOT EXISTS idx_transactions_debit ON transactions(debit_account_id);
CREATE INDEX IF NOT EXISTS idx_transactions_credit ON transactions(credit_account_id);

-- Вставка базовых счетов РСБУ
INSERT OR IGNORE INTO accounts (code, name, type) VALUES
('50', 'Касса', 0),
('51', 'Расчетные счета', 0),
('52', 'Валютные счета', 0),
('60', 'Расчеты с поставщиками и подрядчиками', 2),
('62', 'Расчеты с покупателями и заказчиками', 2),
('70', 'Расчеты с персоналом по оплате труда', 2),
('80', 'Уставный капитал', 1),
('90', 'Продажи', 1),
('91', 'Прочие доходы и расходы', 2);

-- Таблица шаблонов проводок
CREATE TABLE IF NOT EXISTS transaction_templates (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,                    -- Название шаблона
    description TEXT,                      -- Описание
    debit_account_id INTEGER NOT NULL,     -- Счет дебета
    credit_account_id INTEGER NOT NULL,    -- Счет кредита
    amount DECIMAL(15,2),                  -- Сумма (если 0 - вводится вручную)
    is_amount_fixed BOOLEAN DEFAULT 0,     -- Фиксированная сумма?
    document_prefix TEXT,                  -- Префикс номера документа
    counterparty_id INTEGER,               -- Контрагент по умолчанию
    frequency TEXT,                        -- Частота: once, daily, weekly, monthly, yearly
    last_executed DATE,                    -- Когда последний раз выполнялся
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    FOREIGN KEY (debit_account_id) REFERENCES accounts(id),
    FOREIGN KEY (credit_account_id) REFERENCES accounts(id),
    FOREIGN KEY (counterparty_id) REFERENCES counterparties(id)
);

-- Добавляем базовые шаблоны
INSERT OR IGNORE INTO transaction_templates (name, description, debit_account_id, 
    credit_account_id, amount, is_amount_fixed, document_prefix) VALUES
('Начисление зарплаты', 'Ежемесячное начисление заработной платы',
    (SELECT id FROM accounts WHERE code = '70'),
    (SELECT id FROM accounts WHERE code = '50'),
    0, 0, 'ЗП'),
('Оплата поставщику', 'Оплата товаров/услуг поставщику',
    (SELECT id FROM accounts WHERE code = '60'),
    (SELECT id FROM accounts WHERE code = '51'),
    0, 0, 'ОПЛ'),
('Поступление от покупателя', 'Оплата от покупателя',
    (SELECT id FROM accounts WHERE code = '51'),
    (SELECT id FROM accounts WHERE code = '62'),
    0, 0, 'ПОСТ');
