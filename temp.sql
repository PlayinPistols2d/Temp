WITH RECURSIVE task_hierarchy AS (
    -- Anchor member: Select all root tasks by post_id in priority order
    SELECT 
        id,
        name,
        type,
        parent_operation_id,
        priority,
        post_id,
        1 AS level
    FROM operations
    WHERE parent_operation_id IS NULL AND post_id = :post_id
    ORDER BY priority

    UNION ALL

    -- Recursive member: Select all child tasks, following the hierarchy and maintaining priority order
    SELECT 
        o.id,
        o.name,
        o.type,
        o.parent_operation_id,
        o.priority,
        o.post_id,
        th.level + 1 AS level
    FROM operations o
    INNER JOIN task_hierarchy th ON o.parent_operation_id = th.id
    WHERE o.post_id = :post_id
    ORDER BY o.priority
)

-- Final select to get all tasks in the desired order
SELECT * 
FROM task_hierarchy
ORDER BY level, priority;