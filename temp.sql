WITH RECURSIVE task_hierarchy AS (
    -- Anchor member: Select all root tasks with no parent_operation_id (NULL) and for a specific post_id
    SELECT 
        id,
        name,
        type,
        parent_operation_id,
        priority,
        post_id,
        id AS root_id,
        0 AS depth
    FROM operations
    WHERE parent_operation_id IS NULL AND post_id = :post_id

    UNION ALL

    -- Recursive member: Select all child tasks, linking them to their parent tasks
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
    WHERE o.post_id = :post_id
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
    CASE WHEN depth = 0 THEN priority ELSE th.parent_operation_id END ASC,  -- Root tasks first by priority
    root_id ASC,  -- Group by root task
    depth ASC,    -- Ensure that parent tasks appear before their children
    priority ASC; -- Order by priority within each level
