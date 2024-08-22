WITH params AS (
    -- CTE to store the post_number as a variable
    SELECT :post_number AS post_number, :start_date AS start_date, :end_date AS end_date  -- Add start_date and end_date parameters
),
operations_in_shift AS (
    -- Find all operations linked to shifts within the specified date range
    SELECT o.id
    FROM operations o
    INNER JOIN shift_to_operation sto ON o.id = sto.operation_id
    INNER JOIN shifts s ON sto.shift_id = s.id
    INNER JOIN params ON s.start_date >= params.start_date AND s.end_date <= params.end_date
),
task_hierarchy AS (
    -- First step: Select all root tasks with parent_operation_id = NULL and matching post number,
    -- and ensure they are not in the date range specified above.
    SELECT 
        o.id,
        o.name,
        t.name AS type_name,
        g.name AS group_name,
        o.parent_operation_id,
        o.priority,
        p.number AS post_number,
        o.id AS root_id,
        0 AS depth
    FROM operations o
    INNER JOIN posts p ON o.post_id = p.id
    INNER JOIN params ON p.number = params.post_number
    LEFT JOIN types t ON o.type_id = t.id
    LEFT JOIN groups g ON o.group_id = g.id
    WHERE o.parent_operation_id IS NULL
      AND o.id NOT IN (SELECT id FROM operations_in_shift)  -- Exclude operations in the specified shift date range

    UNION ALL

    -- Recursive step: Select all child tasks, linking them to their parent tasks
    SELECT 
        o.id,
        o.name,
        t.name AS type_name,
        g.name AS group_name,
        o.parent_operation_id,
        o.priority,
        p.number AS post_number,
        th.root_id,
        th.depth + 1 AS depth
    FROM operations o
    INNER JOIN task_hierarchy th ON o.parent_operation_id = th.id
    LEFT JOIN types t ON o.type_id = t.id
    LEFT JOIN groups g ON o.group_id = g.id
    LEFT JOIN posts p ON o.post_id = p.id
    WHERE o.id NOT IN (SELECT id FROM operations_in_shift)  -- Ensure child tasks are also not within the date range
)

-- Final query to order tasks in the required hierarchical structure
SELECT 
    id, 
    name, 
    type_name, 
    parent_operation_id, 
    priority, 
    group_name, 
    post_number
FROM task_hierarchy
ORDER BY 
    root_id ASC,      -- Group by root task
    depth ASC,        -- Ensure that parent tasks appear before their children
    priority ASC;     -- Order by priority within each level