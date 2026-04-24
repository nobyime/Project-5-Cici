#version 330

uniform sampler2D texture0;
uniform vec2 lightPosition;
uniform vec2 godLightPosition;

in vec2 fragTexCoord;
in vec2 fragPosition;

out vec4 finalColor;

const float PLAYER_LINEAR_TERM    = 0.00003;
const float PLAYER_QUADRATIC_TERM = 0.0005;

const float GOD_LINEAR_TERM       = 0.00003;
const float GOD_QUADRATIC_TERM    = 0.0001;

const float MIN_BRIGHTNESS = 0.17;

float attenuate(float dist, float linearTerm, float quadraticTerm)
{
    float attenuation = 1.0 / (1.0 +
                               linearTerm * dist +
                               quadraticTerm * dist * dist);

    return attenuation;
}

void main()
{
    vec4 color = texture(texture0, fragTexCoord);

    float playerDist = distance(lightPosition, fragPosition);
    float godDist    = distance(godLightPosition, fragPosition);

    float playerBrightness = attenuate(playerDist, PLAYER_LINEAR_TERM, PLAYER_QUADRATIC_TERM);
    float godBrightness    = attenuate(godDist, GOD_LINEAR_TERM, GOD_QUADRATIC_TERM);

    float brightness = max(playerBrightness, godBrightness);
    brightness = max(brightness, MIN_BRIGHTNESS);
    brightness = min(brightness, 1.0);

    finalColor = vec4(color.rgb * brightness, color.a);
}