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

-- Final select to get all tasks in the desired hierarchical order
SELECT * 
FROM task_hierarchy
ORDER BY 
    CASE 
        WHEN parent_operation_id IS NULL THEN id  -- Root tasks come first, ordered by their ID
        ELSE parent_operation_id  -- Child tasks are grouped under their parent
    END,
    parent_operation_id,  -- Ensures correct grouping under each parent
    priority;  -- Orders by priority within each parent group