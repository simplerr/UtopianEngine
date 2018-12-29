add_test_assets = function()
    debug_print("Loading test scene from Lua...")

    -- Grass and flowers
    for i=0, 70 do
        add_asset(i, (i / 10) * 100, 0, (i % 10) * 100, 1)
    end
end

generate_random_foliage = function()
    debug_print("Generating random foliage from Lua...")

    math.randomseed(os.time())
    local range = 1000

    -- Trees
    for i=0, 40 do
        local asset_id = math.random(0, 24)
        local x = math.random(-range, range)
        local z = math.random(-range, range)
        local scale = math.random(10, 80) / 100
        add_asset(71, x, 0, z, scale)
        -- add_asset(i, (i / 10) * 100, 0, (i % 10) * 100, 1)
    end

    -- Grass and flowers
    for i=0, 200 do
        local asset_id = math.random(0, 24)
        local x = math.random(-range, range)
        local z = math.random(-range, range)
        add_asset(asset_id, x, 0, z, 1)
    end

    -- Rocks
    for i=0, 20 do
        local asset_id = 91 --math.random(0, 24)
        local x = math.random(-range, range)
        local z = math.random(-range, range)
        local scale = math.random(1, 20) / 100
        add_asset(asset_id, x, 0, z, scale)
    end

    -- Bushes
    for i=0, 70 do
        local asset_id = 133 --math.random(0, 24)
        local x = math.random(-range, range)
        local z = math.random(-range, range)
        local scale = 1--math.random(1, 20) / 100
        add_asset(asset_id, x, 0, z, scale)
    end
end

load_foliage = function()
    debug_print("Loading assets from Lua...")
    --add_test_assets()
    generate_random_foliage()
    add_asset(0, 0, 0, 0, 1)
end