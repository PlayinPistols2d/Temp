WITH first_insert AS (
    INSERT INTO your_table (column1, column2)
    VALUES ('value1', 'value2')
    RETURNING id AS first_id
),
second_insert AS (
    INSERT INTO your_table (column1, column2, parent_id)
    VALUES ('value3', 'value4', (SELECT first_id FROM first_insert))
    RETURNING id AS second_id
),
third_insert AS (
    INSERT INTO your_table (column1, column2, parent_id)
    VALUES ('value5', 'value6', (SELECT second_id FROM second_insert))
    RETURNING id AS third_id
)
INSERT INTO final_table (foreign_key, column)
SELECT third_id, 'final_value'
FROM third_insert;