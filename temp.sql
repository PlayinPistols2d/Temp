WITH RECURSIVE task_hierarchy AS (
    -- Anchor member: Select all root tasks by post_id
    SELECT 
        id,
        name,
        type,
        parent_operation_id,
        priority,
        post_id
    FROM operations
    WHERE parent_operation_id IS NULL AND post_id = :post_id

    UNION ALL

    -- Recursive member: Select all child tasks recursively
    SELECT 
        o.id,
        o.name,
        o.type,
        o.parent_operation_id,
        o.priority,
        o.post_id
    FROM operations o
    INNER JOIN task_hierarchy th ON o.parent_operation_id = th.id
    WHERE o.post_id = :post_id
)

-- Final select to get all tasks in the desired order
SELECT * 
FROM task_hierarchy
ORDER BY COALESCE(parent_operation_id, id), priority;