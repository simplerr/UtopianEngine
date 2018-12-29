generate_random_foliage = function(instancing)
    if instancing then
        debug_print("Generating random instanced foliage from Lua...")
    else
        debug_print("Generating random foliage from Lua...")
    end

    math.randomseed(os.time())
    local range = 2000

    -- Trees
    for i=0, 100 do
        local range = 3000
        local asset_id = math.random(0, 24)
        local x = math.random(-range, range)
        local z = math.random(-range, range)
        local y = get_terrain_height(-x, -z) -- Note: Negative signs
        local scale = math.random(10, 80) / 100
        add_asset(71, x, y, z, 180, 0, 0, scale, instancing)
        -- add_asset(i, (i / 10) * 100, 0, (i % 10) * 100, 1)
    end

    -- Grass and flowers
    for i=0, 3000 do
        local asset_id = math.random(0, 24)
        local x = math.random(-range, range)
        local z = math.random(-range, range)
        local y = get_terrain_height(-x, -z) -- Note: Negative signs
        add_asset(asset_id, x, y, z, 180, 0, 0, 1, instancing)
    end

    -- Rocks
    for i=0, 20 do
        local asset_id = 91 --math.random(0, 24)
        local x = math.random(-range, range)
        local z = math.random(-range, range)
        local y = get_terrain_height(-x, -z) -- Note: Negative signs
        local scale = math.random(1, 20) / 100
        add_asset(asset_id, x, y, z, 180, 0, 0, scale, instancing)
    end

    -- Bushes
    for i=0, 70 do
        local asset_id = 133 --math.random(0, 24)
        local x = math.random(-range, range)
        local z = math.random(-range, range)
        local y = get_terrain_height(-x, -z) -- Note: Negative signs
        local scale = 1--math.random(1, 20) / 100
        add_asset(asset_id, x, y, z, 180, 0, 0, scale, instancing)
    end
end

get_terrain_height = function(x, z)
    local frequency = (1.0 / 33000.0)
    local amplitude = 2000.0
    local octaves = 8;
    local height = 0.0

    for i=0, octaves do
        height = height + get_noise(x * frequency, 0, z * frequency) * amplitude
		amplitude = amplitude * 0.5
		frequency = frequency * 2
    end

    return height;
end

load_foliage = function()
    debug_print("Loading assets from Lua...")
    clear_instance_groups()
    generate_random_foliage(true)
    get_terrain_height(15000, 1000)
    --instancing_testing()
    build_instance_buffers()
end