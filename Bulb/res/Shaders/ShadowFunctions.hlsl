static const uint poissonDiskSamples = 32;
static const float2 poissonDisk2D[poissonDiskSamples] = {
    float2( 0.0f, 0.0f ), 
    float2( 0.8048486986946776f,  -0.5852050849392686f   ),    
    float2(-0.9958393427837682f,   0.036113738734521734f ),  
    float2( 0.9072570718270919f,   0.40734700083389624f  ),    
    float2(-0.22789385343283575f,  0.9644404847581753f   ),   
    float2(-0.10689271592202236f, -0.954188338841789f    ),   
    float2(-0.48719337104285393f, -0.42854765804837486f  ), 
    float2( 0.4400381238393658f,   0.728434169786693f    ),      
    float2(-0.6387109841365166f,   0.562209120212132f    ),     
    float2( 0.634731997690473f,   -0.05839784444054089f  ),    
    float2( 0.20747687061341666f, -0.45928201536281393f  ),  
    float2( 0.4157658097156909f,   0.30202918239243165f  ),    
    float2(-0.1906360581448456f,   0.4826355463106642f   ),    
    float2( 0.3134312467124866f,  -0.8695656411272433f   ),    
    float2(-0.4621244966594415f,   0.034149766201863836f ),  
    float2(-0.9333946320259354f,  -0.35319339389923116f  ),  
    float2(-0.513692645101771f,   -0.8400632705059478f   ),    
    float2( 0.9507042255582802f,  -0.22996329319736644f  ),   
    float2(-0.1299055116233461f,  -0.45888900214621164f  ),  
    float2(-0.777115314506443f,   -0.6055347842708125f   ),
    float2( 0.15561938048113194f,  0.5497352288871444f   ),
    float2( 0.1712754063845962f,   0.9805812446301038f   ),
    float2( 0.49633530791493974f, -0.3394307591564362f   ),
    float2( 0.2909022130270505f,  -0.11290995348313707f  ),
    float2(-0.8982917058001596f,   0.3368474679600616f   ),
    float2(-0.25066771849259145f, -0.16150164929802732f  ),
    float2(-0.2334001875946011f,   0.20550212549089877f  ),
    float2(-0.3852498720717099f,   0.6910218613664254f   ),
    float2( 0.08092990179776498f, -0.720619256823693f    ),
    float2( 0.5518520067173671f,  -0.6806660242989737f   ),
    float2(-0.634849590405609f,    0.2817332624087997f   ),
    float2(-0.7317371150804994f,  -0.11790178286328051f  ),
};
static const float3 poissonDisk3D[poissonDiskSamples] = {
    float3( 0.0f, 0.0f, 0.0f ),
    float3( 0.2970119350527858f, 0.9137044248167985f, -0.2693162154312882f ),
    float3(-0.5171754761255204f, -0.709248152150239f, 0.46867384029139963f ),
    float3(-0.44538581381036385f, 0.28360623328457013f, -0.8441223927872976f ),
    float3( 0.9810220725274866f, -0.03544184377706667f, 0.17332364083523577f ),
    float3(-0.9740271565497202f, 0.15618226999047652f, 0.005380007988301553f ),
    float3( 0.5613422345001199f, -0.48618871047817114f, -0.6112433033660324f ),
    float3(-0.23843837356943492f, 0.02383594171908807f, 0.9418022313791492f ),
    float3( 0.13896078198464487f, 0.7750322892150238f, 0.5611438228329888f ),
    float3(-0.3944238993375644f, -0.5203523037866771f, -0.580587962468195f ),
    float3( 0.258348240119418f, -0.7710682808090137f, 0.4040138356452848f ),
    float3(-0.3788265054940143f, 0.6283224441938055f, 0.04380878402602242f ),
    float3( 0.5851829658312667f, 0.17123641719530672f, 0.7913176109387675f ),
    float3( 0.7800894810307873f, 0.4223602536913283f, -0.42843161836658683f ),
    float3( 0.022100030903691288f, -0.9574684087141979f, -0.21196897809182866f ),
    float3( 0.28087695199073925f, 0.3716651489529634f, -0.7877022660088109f ),
    float3(-0.7333323577804098f, -0.12035806808429757f, 0.6201782413736597f ),
    float3(-0.13545667538302653f, -0.49555732176214096f, 0.833756018374939f ),
    float3( 0.08934321061509666f, -0.1363771999642559f, -0.65334642691584f ),
    float3( 0.5063477941713882f, 0.438095355895574f, 0.16041913115059864f ),
    float3( 0.6831890469255234f, -0.6848472827183825f, 0.04422258099795258f ),
    float3(-0.8909167124004707f, -0.43741388763160216f, -0.04101949447077831f ),
    float3(-0.6456120970369887f, 0.33508298363644984f, -0.36794591506542956f ),
    float3(-0.642834063259088f, 0.40577414602941736f, 0.5205182035837411f ),
    float3( 0.33615414783357084f, -0.29141800057456785f, 0.6676744591274754f ),
    float3( 0.5225751275161967f, -0.07081083769194643f, -0.15914862768827528f ),
    float3(-0.12108896280875198f, 0.6040323650803143f, -0.43918371409069085f ),
    float3( 0.18732151744880107f, -0.4569176464322416f, -0.17696909897077215f ),
    float3(-0.502620567981614f, -0.8635201089075424f, -0.037891317515832953f ),
    float3(-0.3899037751374441f, -0.40756585831125985f, -0.0460740097707099f ),
    float3( 0.1403252738408343f, 0.2225453510387535f, 0.41550032815707305f ),
    float3( 0.6121164036106788f, 0.05820146081120524f, -0.7132630308929806f ),
};

static const float offset = 1.0f / 50.0f; //Chosen by experiementation, maybe something we'd want to control with a constant buffer in future
static const float lightSize = 0.3f; //NOTE: Currently we only have a point and directional lights (an no area lights) so our light size is hard coded

float2 rotate(float2 pos, float angle){
    float2 rotated;
    rotated.x = (cos(angle) * pos.x) - (sin(angle) * pos.y);
    rotated.y = (sin(angle) * pos.x) + (cos(angle) * pos.y);
    return rotated;
}

float estimatePenumbraSize(float lightSize, float recieverDepth, float averageBlockerDepth){
    const float penumbra = lightSize * (recieverDepth - averageBlockerDepth) / averageBlockerDepth;  
    return penumbra;
}

//Directional Shadow maps
float getAverageBlockerDistance(Texture2DArray tex, SamplerState state, float3 projectionCoords, int lightIndex, float offset){
    int blockers = 0;
    float avgerageBlockerDist = 0.0f;
    
    const float currentDepth = projectionCoords.z;
    
    for(int i = 0; i < poissonDiskSamples; ++i){
        const float2 rotatedOffset = rotate(poissonDisk2D[i] * offset, radians(45));
        const float2 sampleLocation = projectionCoords.xy + rotatedOffset;
        
        const float depth = tex.Sample(state, float3(sampleLocation, lightIndex)).r;
        if(depth < currentDepth){
            blockers++;
            avgerageBlockerDist += depth;
        }
    }
    
    avgerageBlockerDist /= blockers;
    
    return avgerageBlockerDist;
}

float GenerateShadow_PCF(Texture2DArray tex, SamplerState state, float3 projectionCoords, int lightIndex, float shadowOffsetBias, float2 texelSize){
    const float currentDepth = projectionCoords.z;
	
	float shadow = 0.0f;
	for(uint i = 0; i < poissonDiskSamples; ++i){
		const float2 sampleLocation = projectionCoords.xy + poissonDisk2D[i] * texelSize;
		float closestDepth = tex.Sample(state, float3(sampleLocation, lightIndex)).r;
		shadow += currentDepth - shadowOffsetBias > closestDepth ? 1.0f : 0.0f;
	}	
	shadow /= poissonDiskSamples;
    
    return shadow;
}

float GenerateShadow_PCSS(Texture2DArray tex, SamplerState state, float3 projectionCoords, int lightIndex, float shadowOffsetBias, float2 texelSize){   
    float averageBlockerDistance = getAverageBlockerDistance(tex, state, projectionCoords, lightIndex, offset);
    
    if(averageBlockerDistance == 0.0f){
        return 0.0f;
    }else{
        const float penumbraSize = estimatePenumbraSize(lightSize, projectionCoords.z, averageBlockerDistance);
        const float shadow = GenerateShadow_PCF(tex, state, projectionCoords, lightIndex, shadowOffsetBias, penumbraSize);
        
        return shadow;
    }
}

//Point shadow maps
float GenerateShadow_PCF(TextureCubeArray tex, SamplerState state, float3 fragToLight, float farPlane, int lightIndex, float shadowOffsetBias, float diskRadius){
    const float currentDepth = length(fragToLight);
    
    float shadow = 0.0;
	for(uint i = 0; i < poissonDiskSamples; ++i){
		const float3 sampleLocation = fragToLight + poissonDisk3D[i] * diskRadius;
		float closestDepth = tex.Sample(state, float4(sampleLocation, lightIndex)).r;
		closestDepth *= farPlane;
		if(currentDepth - shadowOffsetBias > closestDepth){
			shadow += 1.0;
		}
	}
	shadow /= poissonDiskSamples;
    
    return shadow;
}