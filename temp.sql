DO $$
DECLARE
    new_id INTEGER; -- Declaring the variable
BEGIN
    -- Insert query to capture the returning ID
    INSERT INTO my_table (column1, column2)
    VALUES ('value1', 'value2')
    RETURNING id INTO new_id;

    -- Use the variable in subsequent queries
    INSERT INTO another_table (foreign_key_column, other_column)
    VALUES (new_id, 'other_value1');

    INSERT INTO yet_another_table (related_id, description)
    VALUES (new_id, 'some description');
END $$;