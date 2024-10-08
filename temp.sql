WITH RECURSIVE operations_hierarchy AS (
    -- Anchor member: Select all parent operations linked to the specified shift (where parent_operation_id IS NULL)
    SELECT 
        o.id,
        o.name,
        t.name AS type_name,
        g.name AS group_name,
        o.parent_operation_id,
        o.priority,
        p.number AS post_number,
        o.id AS root_id,  -- Use root_id to group by the root parent operation
        0 AS depth
    FROM operations o
    INNER JOIN shift_to_operation sto ON o.id = sto.operation_id
    INNER JOIN types t ON o.type_id = t.id
    LEFT JOIN groups g ON o.group_id = g.id
    LEFT JOIN posts p ON o.post_id = p.id
    WHERE sto.shift_id = :shift_id  -- Filtering by the shift ID
      AND o.parent_operation_id IS NULL  -- Selecting only parent operations (no parent)

    UNION ALL

    -- Recursive member: Select all child operations linked to the parent operations
    SELECT 
        o.id,
        o.name,
        t.name AS type_name,
        g.name AS group_name,
        o.parent_operation_id,
        o.priority,
        p.number AS post_number,
        th.root_id,  -- Use the same root_id from the parent operation
        th.depth + 1 AS depth  -- Increase depth level for child operations
    FROM operations o
    INNER JOIN operations_hierarchy th ON o.parent_operation_id = th.id  -- Recursively join on parent_operation_id
    INNER JOIN types t ON o.type_id = t.id
    LEFT JOIN groups g ON o.group_id = g.id
    LEFT JOIN posts p ON o.post_id = p.id
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
FROM operations_hierarchy
ORDER BY 
    root_id ASC,      -- Group by root parent task
    depth ASC,        -- Ensure that parent tasks appear before their children
    priority ASC;     -- Order by priority within each level