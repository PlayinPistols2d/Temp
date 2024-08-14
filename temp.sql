WITH params AS (
    -- This CTE will store the post_number as a "variable"
    SELECT :post_number AS post_number
),
task_hierarchy AS (
    -- First step: Select all root tasks with parent_operation_id = NULL and matching post number
    SELECT 
        o.id,
        o.name,
        t.name AS type_name,
        g.name AS group_name,
        o.parent_operation_id,
        o.priority,
        o.post_id,
        o.id AS root_id,
        0 AS depth
    FROM operations o
    INNER JOIN posts p ON o.post_id = p.id
    INNER JOIN params ON p.post_number = params.post_number
    
    -- Joining to the many-to-many table and then to the types table to get the exact type name
    LEFT JOIN opt ON o.id = opt.operation_id
    LEFT JOIN types t ON opt.type_id = t.id

    -- Joining to the many-to-many table and then to the groups table to get the actual group name
    LEFT JOIN operation_user_groups oug ON o.id = oug.operation_id
    LEFT JOIN groups g ON oug.group_id = g.id

    WHERE o.parent_operation_id IS NULL

    UNION ALL

    -- Recursive step: Select all child tasks, linking them to their parent tasks
    SELECT 
        o.id,
        o.name,
        t.name AS type_name,
        g.name AS group_name,
        o.parent_operation_id,
        o.priority,
        o.post_id,
        th.root_id,
        th.depth + 1 AS depth
    FROM operations o
    INNER JOIN task_hierarchy th ON o.parent_operation_id = th.id
    
    -- Joining to the many-to-many table and then to the types table to get the exact type name
    LEFT JOIN opt ON o.id = opt.operation_id
    LEFT JOIN types t ON opt.type_id = t.id

    -- Joining to the many-to-many table and then to the groups table to get the actual group name
    LEFT JOIN operation_user_groups oug ON o.id = oug.operation_id
    LEFT JOIN groups g ON oug.group_id = g.id

    WHERE o.post_id = th.post_id
)

-- Final query to order tasks in the required hierarchical structure
SELECT 
    id, 
    name, 
    type_name, 
    group_name, 
    parent_operation_id, 
    priority, 
    post_id
FROM task_hierarchy
ORDER BY 
    root_id ASC,      -- Group by root task
    depth ASC,        -- Ensure that parent tasks appear before their children
    priority ASC;     -- Order by priority within each level