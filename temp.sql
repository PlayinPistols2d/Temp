BEGIN;

-- Создаем временную таблицу для хранения ID
CREATE TEMP TABLE temp_ids (id INTEGER);

-- Добавляем первые 10 вставок и сохраняем ID
INSERT INTO your_table (column1, column2)
VALUES ('value1', 'value2')
RETURNING id INTO temp_ids;

INSERT INTO your_table (column1, column2)
VALUES ('value3', 'value4')
RETURNING id INTO temp_ids;

-- Дополнительные запросы, использующие сохраненные ID
INSERT INTO another_table (foreign_key, column)
SELECT id, 'valueX'
FROM temp_ids;

-- Другие действия, которые требуют ID
UPDATE another_table
SET some_column = 'updated_value'
WHERE foreign_key IN (SELECT id FROM temp_ids);

-- Очистка временной таблицы (если не требуется более)
DROP TABLE temp_ids;

COMMIT;