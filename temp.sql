WITH params AS (
    -- This CTE will store the post_number as a "variable"
    SELECT :post_number AS post_number
),
task_hierarchy AS (
    -- First step: Select all root tasks with parent_operation_id = NULL and matching post number
    SELECT 
        o.id,
        o.name,
        o.type,
        o.parent_operation_id,
        o.priority,
        o.post_id,
        o.id AS root_id,
        0 AS depth
    FROM operations o
    INNER JOIN posts p ON o.post_id = p.id
    INNER JOIN params ON p.post_number = params.post_number
    WHERE o.parent_operation_id IS NULL

    UNION ALL

    -- Recursive step: Select all child tasks, linking them to their parent tasks
    SELECT 
        o.id,
        o.name,
        o.type,
        o.parent_operation_id,
        o.priority,
        o.post_id,
        th.root_id,
        th.depth + 1 AS depth
    FROM operations o
    INNER JOIN task_hierarchy th ON o.parent_operation_id = th.id
    WHERE o.post_id = th.post_id
)

-- Final query to order tasks in the required hierarchical structure
SELECT 
    id, 
    name, 
    type, 
    parent_operation_id, 
    priority, 
    post_id
FROM task_hierarchy
ORDER BY 
    root_id ASC,      -- Group by root task
    depth ASC,        -- Ensure that parent tasks appear before their children
    priority ASC;     -- Order by priority within each level