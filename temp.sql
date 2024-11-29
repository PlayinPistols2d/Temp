BEGIN;

-- Шаг 1: Инсерт в первую таблицу
INSERT INTO first_table (column1, column2)
VALUES ('value1', 'value2');

-- Шаг 2: Получение последнего вставленного ID
SELECT LASTVAL() INTO last_inserted_id;

-- Шаг 3: Использование этого ID для инсерта во вторую таблицу
INSERT INTO second_table (first_table_id, other_column)
VALUES (last_inserted_id, 'value_for_second_table');

COMMIT;