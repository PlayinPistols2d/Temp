CREATE OR REPLACE FUNCTION insert_and_process(
    static_arg1 TEXT,
    static_arg2 TEXT,
    static_arg3 TEXT,
    static_arg4 TEXT,
    input_data TEXT[],
    insert_count INT
) RETURNS VOID AS $$
DECLARE
    new_id INT;
    i INT;
BEGIN
    -- Insert static data into the first table
    INSERT INTO static_table (column1, column2, column3, column4)
    VALUES (static_arg1, static_arg2, static_arg3, static_arg4)
    RETURNING id INTO new_id;

    -- Perform multiple inserts using the returned id
    FOR i IN 1..insert_count LOOP
        INSERT INTO related_table (static_id, data)
        VALUES (new_id, input_data[i]);
    END LOOP;
END;
$$ LANGUAGE plpgsql;